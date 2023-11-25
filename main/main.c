#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "mongoose.h"
#include "nvs_flash.h"
#include "wifi.h"

#define QUOTE(...) #__VA_ARGS__
#define NUM_PINS 10

struct {
    int modes[NUM_PINS];
} state;

void save_config(int config[NUM_PINS]) {
    printf("writing..\n");
    //if (remove("config") == 0) {
    //    printf("Deleted successfully");
    //} else {
    //    printf("Unable to delete the file");
    //}

    FILE *fptr = fopen("config", "wb");
    if (fptr == NULL) {
        printf("Error opening config file\n");
        return;
    }

    fwrite(config, sizeof(int), NUM_PINS, fptr);
    fclose(fptr);
}

void read_config(int config[NUM_PINS]) {
    FILE *fptr = fopen("config", "rb");
    if (fptr == NULL) {
        for (int i=0; i<NUM_PINS; ++i) {
            config[i] = 0;
        }
        return;
    }

    int ret = fread(config, sizeof(int), NUM_PINS, fptr);
    if (ret != NUM_PINS) {
        printf("Error reading config file (%d)\n", ret);
    }

    fclose(fptr);

    for (int i=0; i<NUM_PINS; ++i) {
        printf("%d - %d\n", i, config[i]);
    }
}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    mg_http_reply(
        conn, 
        200, 
        "Content-Type: text/html\r\n", 
        QUOTE(
            <!DOCTYPE html>
            <html lang="en">
                <head>
                    <title>Board Configuration</title>
                    <style>
                        form {
                            display: grid;
                            grid-template-columns: 1fr 2fr;
                            gap: 10px;
                            width: 300px;
                            margin-top: 30px;
                            margin-left: auto;
                            margin-right: auto;
                        }
                    </style>
                </head>
                <body>
                    <form method="post" action="set-config">
                        <label>Pin 1</label>
                        <select name="1">
                            <option value="0">Disabled</option>
                            <option value="1">Digital Input</option>
                            <option value="2">Digital Output</option>
                            <option value="3">Analog Input</option>
                            <option value="4">Analog Output</option>
                        </select>
                        <label>Pin 2</label>
                        <select name="2">
                            <option value="0">Disabled</option>
                            <option value="1">Digital Input</option>
                            <option value="2">Digital Output</option>
                            <option value="3">Analog Input</option>
                            <option value="4">Analog Output</option>
                        </select>
                        <input type="submit" value="Save"/>
                    </form>
                </body>
            </html>
        )
    );
};

static void handle_set_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct mg_str body = hm->body;
    
    int config[NUM_PINS];
    for (int i=0; i<NUM_PINS; ++i) {
        char key[3];
        snprintf(key, 3, "%d", i);

        int mode = 0;
        struct mg_str val = mg_http_var(body, mg_str(key));
        if (val.len > 0) {
            char* end;
            mode = strtol(val.ptr, &end, 10);

        }
        config[i] = mode;
    }

    save_config(config);

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
    //Initialize NVS
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


