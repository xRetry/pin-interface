#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "mongoose.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "utils.h"

#define QUOTE(...) #__VA_ARGS__
#define NUM_PINS 10
#define NUM_CHARS_JS NUM_PINS * 20
#define NUM_CONTENT NUM_CHARS_JS + 1000
#define JS_TEMPLATE "\
    <!DOCTYPE html> \
    <html lang=\"en\"> \
        <head> \
            <title>Board Configuration</title> \
            <style> \
                form { \
                    display: grid; \
                    grid-template-columns: 1fr 2fr; \
                    gap: 10px; \
                    width: 300px; \
                    margin-top: 30px; \
                    margin-left: auto; \
                    margin-right: auto; \
                } \
            </style> \
        </head> \
        <body> \
            <form method=\"post\" action=\"set-config\"> \
                <input type=\"submit\" value=\"Save\"/> \
            </form> \
        <script> \
            %s \
            const MODE_DESCS = ['Disabled', 'Digital Input', 'Digital Output', 'Analog Input', 'Analog Output']; \
\
            let form = document.querySelector('form'); \
            for (const [num, modes, mode] of PINS.reverse()) { \
                let select = `<select name=\"${num}\">`; \
                let i = 0; \
                for (const m of modes.toString(2)) { \
                    const sel = i === mode ? ' selected' : ''; \ 
                    if (m > 0) { select += `<option value=\"${i}\"${sel}>${MODE_DESCS[i]}</option>`; } \
                    ++i; \
                } \
\
                form.insertAdjacentHTML('afterbegin', select+'</select>'); \
                form.insertAdjacentHTML('afterbegin', `<label>Pin ${num}</label>`); \
            } \
        </script> \
        </body> \
    </html> \
"

struct {
    int modes[NUM_PINS];
} state;

void save_config(int config[NUM_PINS]) {
    esp_err_t err = utils_fwrite_binary("config", config, sizeof(int)*NUM_PINS);
    if (err != ESP_OK) {
        printf("Error saving config (%d)\n", err);
    }
}

void read_config(int config[NUM_PINS]) {
    printf("Reading config..\n");
    esp_err_t err = utils_fread_binary("config", config, sizeof(int)*NUM_PINS);
    if (err != ESP_OK) {
        printf("Error reading config file (%d)\n", err);
        for (int i=0; i<NUM_PINS; ++i) {
            config[i] = 0;
        }
    }
}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    char pins[330] = "const PINS = [";

    for (int i=0; i<NUM_PINS; ++i) {
        //TODO(marco): Insert condition for invalid pins
        if (false) { continue; }

        char pin[3];
        snprintf(pin, 3, "%d", i);
        char mode[4];
        snprintf(mode, 4, "%d", state.modes[i]);

        char str[20] = "[";
        strcat(str, pin);
        strcat(str, ",");
        strcat(str, "31");
        strcat(str, ",");
        strcat(str, mode);
        strcat(str, "],");

        strcat(pins, str);
    }
    strcat(pins, "];");


    // Heap allocation necessary to avoid overflow due to limited stack size
    char *content = (char*) malloc((NUM_PINS*20+strlen(JS_TEMPLATE))*sizeof(char));
    if (content == NULL) {
        printf("Allocation failed\n");
        exit(1);
    }
    snprintf(content, 2000, JS_TEMPLATE, pins);

    mg_http_reply(
        conn, 
        200, 
        "Content-Type: text/html\r\n", 
        content
    );

    free(content);
}

static void handle_set_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct mg_str body = hm->body;
    
    for (int i=0; i<NUM_PINS; ++i) {
        char key[3];
        snprintf(key, 3, "%d", i);

        int mode = 0;
        struct mg_str val = mg_http_var(body, mg_str(key));
        if (val.len > 0) {
            char* end;
            mode = strtol(val.ptr, &end, 10);

        }
        state.modes[i] = mode;
    }

    save_config(state.modes);

    mg_http_reply(
        conn, 
        303, // `See Other`
        "Location: /config\r\n", 
        ""
    );
}

static void handle_pins_read(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    printf("read %s", ws_msg->data.ptr);
}

static void handle_pins_write(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    printf("write: %s", ws_msg->data.ptr);
}

static void router(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/config")) {
            handle_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/set-config")) {
            handle_set_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/pins/read")) {
            c->data[0] = 'r';
            mg_ws_upgrade(c, hm, NULL);
        } else if (mg_http_match_uri(hm, "/pins/write")) {
            c->data[0] = 'w';
            mg_ws_upgrade(c, hm, NULL);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/html", "");
        }
    } else if (ev == MG_EV_WS_MSG) {
        if (c->data[0] == 'r') {
            handle_pins_read(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'w') {
            handle_pins_write(c, ev, ev_data, fn_data);
        }
    }
}

void init_state() {
    read_config(state.modes);
}

int app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();

    init_state();

    struct mg_mgr mgr;                                
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", router, NULL);
    for (;;) {
        mg_mgr_poll(&mgr, 300);
    }

    return 0;
}


