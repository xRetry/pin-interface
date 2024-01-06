#include "mongoose.h"
#include "pin_interface.h"

#define READ_TOPIC "pins/read"
#define WRITE_TOPIC "pins/write"
#define QOS 1
#define URL "mqtt://3.120.7.115:8884/clientId-hH6nym9YA6"

static struct mg_connection *mqtt_conn;

static void handle_mqtt(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_OPEN) {
        MG_INFO(("%lu CREATED", c->id));
        // c->is_hexdumping = 1;
    } else if (ev == MG_EV_ERROR) {
        // On error, log error message
        MG_ERROR(("%lu ERROR %s", c->id, (char *) ev_data));
    } else if (ev == MG_EV_MQTT_OPEN) {
        // MQTT connect is successful
        struct mg_str write_topic = mg_str(WRITE_TOPIC), data = mg_str("hello");
        MG_INFO(("%lu CONNECTED to %s", c->id, URL));
        struct mg_mqtt_opts sub_opts;
        memset(&sub_opts, 0, sizeof(sub_opts));
        sub_opts.topic = write_topic;
        sub_opts.qos = QOS;
        mg_mqtt_sub(c, &sub_opts);
        MG_INFO(("%lu SUBSCRIBED to %.*s", c->id, (int) write_topic.len, write_topic.ptr));

        struct mg_str read_topic = mg_str(READ_TOPIC);
        struct mg_mqtt_opts pub_opts;
        memset(&pub_opts, 0, sizeof(pub_opts));
        pub_opts.topic = read_topic;
        pub_opts.message = data;
        pub_opts.qos = QOS, pub_opts.retain = false;
        mg_mqtt_pub(c, &pub_opts);
        MG_INFO(("%lu PUBLISHED %.*s -> %.*s", c->id, (int) data.len, data.ptr, (int) read_topic.len, read_topic.ptr));
    } else if (ev == MG_EV_MQTT_MSG) {
        // When we get echo response, print it
        struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
        MG_INFO(("%lu RECEIVED %.*s <- %.*s", c->id, (int) mm->data.len,
        mm->data.ptr, (int) mm->topic.len, mm->topic.ptr));
    } else if (ev == MG_EV_CLOSE) {
        MG_INFO(("%lu CLOSED", c->id));
        mqtt_conn = NULL;  // Mark that we're closed
    }
    (void) fn_data;
}

static void mqtt_timer(void *arg) {
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    struct mg_mqtt_opts opts = {
        .clean = true,
        .qos = QOS,
        .topic = mg_str(READ_TOPIC),
        .version = 4,
        .message = mg_str("bye")};
    if (mqtt_conn == NULL) mqtt_conn = mg_mqtt_connect(mgr, URL, &opts, handle_mqtt, NULL);
}

void mqtt_client_run(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_timer_add(&mgr, 500, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, mqtt_timer, &mgr);
    while (1) {
        mg_mgr_poll(&mgr, 500);
    }
}
