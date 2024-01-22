#include "mongoose.h"
#include "pin_interface.h"

#define QOS 1

static struct mg_connection *mqtt_conn;

static void mqtt_handle_write_init(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_str write_topic = mg_str(CONFIG_PI_MQTT_WRITE_TOPIC);
    MG_INFO(("%lu CONNECTED to %s", c->id, CONFIG_PI_MQTT_SERVER_URL));

    struct mg_mqtt_opts sub_opts;
    memset(&sub_opts, 0, sizeof(sub_opts));
    sub_opts.topic = write_topic;
    sub_opts.qos = QOS;

    mg_mqtt_sub(c, &sub_opts);
    MG_INFO(("%lu SUBSCRIBED to %.*s", c->id, (int) write_topic.len, write_topic.ptr));
}

static void mqtt_handle_write(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *) ev_data;
    MG_INFO(("%lu RECEIVED %.*s <- %.*s", c->id, (int) msg->data.len,
    msg->data.ptr, (int) msg->topic.len, msg->topic.ptr));

    // The message needs to be a double[PI_NUM_PINS] array
    if (msg->data.len != 4*PI_NUM_PINS) { return; }

    double *vals = (double *) msg->data.ptr;
    for (int i=0; i<PI_NUM_PINS; ++i) {
        pi_exec_pin_op(i, &vals[i]);
    }
}

static void mqtt_handle_read(void *args) {
    if (!mqtt_conn) { return; }

    double vals[PI_NUM_PINS];
    for (int i=0; i<PI_NUM_PINS; ++i) {
        if (PI_IS_ERR(pi_exec_pin_op(i, &vals[i]))) {
            vals[i] = 0;
        }
    }

    struct mg_str read_topic = mg_str(CONFIG_PI_MQTT_READ_TOPIC);
    struct mg_mqtt_opts pub_opts;

    struct mg_str data = { .ptr = (char *) vals, .len = 4*PI_NUM_PINS };
    memset(&pub_opts, 0, sizeof(pub_opts));
    pub_opts.topic = read_topic;
    pub_opts.message = data;
    pub_opts.qos = QOS;
    pub_opts.retain = false;
    mg_mqtt_pub(mqtt_conn, &pub_opts);

    MG_INFO(("%lu PUBLISHED %.*s -> %.*s", mqtt_conn->id, (int) data.len, data.ptr, (int) read_topic.len, read_topic.ptr));
}

static void mqtt_callback(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_OPEN) {
        MG_INFO(("%lu CREATED", c->id));

    } else if (ev == MG_EV_ERROR) {
        MG_ERROR(("%lu ERROR %s", c->id, (char *) ev_data));

    } else if (ev == MG_EV_MQTT_OPEN) {
        mqtt_handle_write_init(c, ev, ev_data, fn_data);

    } else if (ev == MG_EV_MQTT_MSG) {
        mqtt_handle_write(c, ev, ev_data, fn_data);

    } else if (ev == MG_EV_CLOSE) {
        MG_INFO(("%lu CLOSED", c->id));
        mqtt_conn = NULL;  // Mark that we're closed
    }

    (void) fn_data;
}

static void mqtt_handle_connect(void *arg) {
    if (!mqtt_conn) {
        struct mg_mgr *mgr = (struct mg_mgr *) arg;
        struct mg_mqtt_opts opts = {
            .clean = true,
            .qos = QOS,
            .topic = mg_str(CONFIG_PI_MQTT_READ_TOPIC),
            .version = 4,
            .message = mg_str("bye")};
        mqtt_conn = mg_mqtt_connect(mgr, CONFIG_PI_MQTT_SERVER_URL, &opts, mqtt_callback, NULL);
    }
}

void mqtt_client_run(void) {
    printf("Starting MQTT client...\n");
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_timer_add(&mgr, 2000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, mqtt_handle_connect, &mgr);
    mg_timer_add(&mgr, 1000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, mqtt_handle_read, &mgr);
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
}
