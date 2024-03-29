#include "mongoose.h"
#include "webserver.h"
#include "constants.h"
#include "pin_interface.h"
#include "utils.h"

#define STRLEN_PIN_NR 3
#define STRLEN_OP_NR 3
#define STRLEN_JSON_ALLOWED ((STRLEN_PIN_NR + 1) * PI_NUM_PINS + 3)
#define STRLEN_JSON_OP (strlen(TEMPLATE_JSON_OP) + STRLEN_JSON_ALLOWED + STRLEN_OP_NR + PI_STRLEN_OP_NAME + 1)
#define STRLEN_JSON_OPS ((STRLEN_JSON_OP + 1) * PI_NUM_OPS + 3)
#define STRLEN_JSON_ACTIVE_ENTRY (STRLEN_PIN_NR + STRLEN_OP_NR + 3)
#define STRLEN_JSON_ACTIVE (PI_NUM_PINS * STRLEN_JSON_ACTIVE_ENTRY + 2)

void ops_as_json(char *json) {
    json[0] = '\0';

    strcat(json, "[");
    for (int i=0; i<PI_NUM_OPS; ++i) {
        char *comma = i == 0 ? "\0" : ",";
        strcat(json, comma);

        char json_allowed[STRLEN_JSON_ALLOWED] = "[";
        for (int j=0; j<PI_PIN_OPS[i].pins_allowed_size; ++j) {
            char *c = j == 0 ? "\0" : ",";
            strcat(json_allowed, c);
            char pin_nr[STRLEN_PIN_NR];
            snprintf(pin_nr, STRLEN_PIN_NR, "%d", PI_PIN_OPS[i].pins_allowed[j]);
            strcat(json_allowed, pin_nr);
        }
        strcat(json_allowed, "]");

        char json_ops[STRLEN_JSON_OPS];
        snprintf(json_ops, STRLEN_JSON_OPS, TEMPLATE_JSON_OP, i, PI_PIN_OPS[i].name, json_allowed);
        strcat(json, json_ops);
    }
    strcat(json, "]");
}

void active_as_json(char *json) {
    json[0] = '\0';

    strcat(json, "{");
    for (int i=0; i<PI_NUM_PINS; ++i) {
        char *comma = i == 0 ? "\0" : ",";
        strcat(json, comma);

        char json_mode[STRLEN_JSON_ACTIVE_ENTRY];
        snprintf(json_mode, STRLEN_JSON_ACTIVE_ENTRY, "\"%d\":%d", i, pi_state.active_op_nrs[i]);
        strcat(json, json_mode);
    }
    strcat(json, "}");
}

static void handle_pins_read(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;

    // TODO(marco): Allocate memory the correct number pins
    double vals[PI_NUM_PINS];
    const long pin_mask = conn->data[1];
    for (int pin_nr=0; pin_nr<PI_NUM_PINS; ++pin_nr) {
        const bool use_pin = (pin_mask >> pin_nr) & 1;
        if (use_pin) {
            pi_exec_pin_op(pin_nr, &vals[pin_nr]);
        } else {
            vals[pin_nr] = 0.;
        }
    }

    for (int i=0; i<PI_NUM_PINS; ++i) {
        printf("%f,", vals[i]);
    }
    printf("\nread %s\n", ws_msg->data.ptr);
}

static void handle_pins_write(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    printf("write: %s", ws_msg->data.ptr);
}

static void handle_pins_json(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    
    int offset, length;
    if ((offset = mg_json_get(ws_msg->data, "$.write", &length)) >= 0) {
        struct mg_str write = mg_str_n(&ws_msg->data.ptr[offset], length);

        struct mg_str key, val;
        offset = 0;
        while ((offset = mg_json_next(write, offset, &key, &val)) > 0) {
            if (val.ptr == NULL) { continue; }

            long pin_nr = 0;
            // The +1 cuts off the `"`
            if ( PI_IS_ERR(utils_string_to_long(key.ptr+1, &pin_nr)) ) { continue; }

            if (pi_state.directions[pin_nr] != PI_WRITE) { continue; }

            double pin_val = 0;
            if ( PI_IS_ERR(utils_string_to_double(val.ptr, &pin_val)) ) { continue; }

            pi_exec_pin_op(pin_nr, &pin_val);
        }
    }

    char response[20*PI_NUM_PINS+3] = "{";

    bool isFirst = true;
    offset = mg_json_get(ws_msg->data, "$.read", &length);
    if (offset >= 0) {
        struct mg_str read = mg_str_n(&ws_msg->data.ptr[offset], length);

        struct mg_str key, val;
        offset = 0;
        while ((offset = mg_json_next(read, offset, &key, &val)) > 0) {
            if (val.ptr == NULL) { continue; }

            long pin_nr = 0;
            if ( PI_IS_ERR(utils_string_to_long(val.ptr, &pin_nr)) ) { continue; }

            if (pi_state.directions[pin_nr] != PI_READ) { continue; }

            double pin_val = 0;
            if ( PI_IS_ERR(pi_exec_pin_op(pin_nr, &pin_val)) ) { continue; }

            char fmt[] = ",\"%d\":%.10g";
            if (isFirst) {
                fmt[0] = ' ';
                isFirst = false;
            }

            char buf[20]; 
            snprintf(buf, sizeof(buf), fmt, pin_nr, pin_val);
            strcat(response, buf);
        }
    }

    strcat(response, " }");

    mg_ws_send(c, response, strlen(response), WEBSOCKET_OP_TEXT);
}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    const int STRLEN_BODY = strlen(TEMPLATE_HTML_CONFIG)+STRLEN_JSON_OPS+STRLEN_JSON_ACTIVE+1;
    char *body;
    if ((body = malloc(sizeof(char)*STRLEN_BODY)) == NULL) {
        printf("Error allocating HTML body");
        return;
    }
    char *ops;
    if ((ops = malloc(sizeof(char)*STRLEN_JSON_OPS)) == NULL) {
        printf("Error allocating json ops");
        return;
    }
    ops_as_json(ops);

    char *active;
    if ((active = malloc(sizeof(char)*STRLEN_JSON_OPS)) == NULL) {
        printf("Error allocating json active");
        return;
    }
    active_as_json(active);

    snprintf(body, STRLEN_BODY, TEMPLATE_HTML_CONFIG, ops, active);

    mg_http_reply(
        conn, 
        200, 
        "Content-Type: text/html\r\n", 
        body
    );

    free(ops);
    free(active);
    free(body);
};

static void handle_api_ops(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *msg = (struct mg_http_message *) ev_data;
    if (mg_strcmp(msg->method, mg_str("GET")) == 0) {
        char json[STRLEN_JSON_OPS];
        ops_as_json(json);
        mg_http_reply(
            conn, 
            200, 
            "Content-Type: application/json\r\n", 
            json
        );
        return;
    }

    mg_http_reply(
        conn, 
        405, 
        "Allow: GET\r\n", 
        "Method Not Allowed"
    );
}

static void handle_api_active(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *msg = (struct mg_http_message *) ev_data;
    if (mg_strcmp(msg->method, mg_str("GET")) == 0) {
        char json[STRLEN_JSON_ACTIVE];
        active_as_json(json);

        mg_http_reply(
            conn, 
            200, 
            "Content-Type: application/json\r\n", 
            json
        );
        return;
    }

    mg_http_reply(
        conn, 
        405, 
        "Allow: GET\r\n", 
        "Method Not Allowed"
    );
}

static void handle_api_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *msg = (struct mg_http_message *) ev_data;
    if (mg_strcmp(msg->method, mg_str("POST")) == 0) {
        struct mg_str body = msg->body;
        
        for (int i=0; i<PI_NUM_PINS; ++i) {
            char key[3];
            snprintf(key, 3, "%d", i);

            int mode = 0;
            struct mg_str val = mg_http_var(body, mg_str(key));
            if (val.len > 0) {
                char* end;
                mode = strtol(val.ptr, &end, 10);
            }
            pi_init_pin_op(i, mode);
        }

        mg_http_reply(
            conn, 
            303, // Status code: `See Other`
            "Location: /config\r\n", 
            "See Other"
        );
    }

    mg_http_reply(
        conn, 
        405, 
        "Allow: POST\r\n", 
        "Method Not Allowed"
    );
}

static void handle_ws_request(struct mg_connection *conn, struct mg_http_message *hm, char direction, const struct mg_str *params) {
    long pins;
    if (utils_string_to_long(params[0].ptr, &pins) != 0) {
        mg_http_reply(conn, 404, "Content-Type: text/html", "");
        return;
    }
    conn->data[0] = direction;
    // pins is bigger than 1 byte, but the overflow is intented
    conn->data[1] = pins;
    mg_ws_upgrade(conn, hm, NULL);
}

static void router(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_str params[1];

        if (mg_http_match_uri(hm, "/config")) {
            handle_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/api/config")) {
            handle_api_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/api/operations")) {
            handle_api_ops(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/api/active")) {
            handle_api_active(c, ev, ev_data, fn_data);
        } else if (mg_match(hm->uri, mg_str("/ws/pins/read/*"), params)) {
            handle_ws_request(c, hm, 'r', params);
        } else if (mg_match(hm->uri, mg_str("/ws/pins/write/*"), params)) {
            handle_ws_request(c, hm, 'w', params);
        } else if (mg_http_match_uri(hm, "/ws/pins")) {
            c->data[0] = 'j';
            mg_ws_upgrade(c, hm, NULL);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/html", "");
        }
    } else if (ev == MG_EV_WS_MSG) {
        if (c->data[0] == 'r') {
            handle_pins_read(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'w') {
            handle_pins_write(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'j') {
            handle_pins_json(c, ev, ev_data, fn_data);
        }
    }
}

void webserver_run(void) {
    printf("Starting HTTP server...\n");
    struct mg_mgr mgr;                                
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", router, NULL);
    for (;;) {
        mg_mgr_poll(&mgr, 300);
    }
}
