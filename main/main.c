#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "mongoose.h"
#include "nvs_flash.h"
#include "wifi.h"

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/api/hello")) {              // On /api/hello requests,
            mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), 1);                   // Send dynamic JSON response
        } else {                                                // For all other URIs,
            mg_http_reply(c, 404, "", "{%m:%d}\n", MG_ESC("status"), 1);                   // Send dynamic JSON response
            //struct mg_http_serve_opts opts = {.root_dir = "."};   // Serve files
            //mg_http_serve_dir(c, hm, &opts);                      // From root_dir
        }
    }
}

void app_main(void) {
    printf("Hello world!\n");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    return;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

    esp_err_t err = esp_wifi_init(&config);
    printf("%d", err);


    return;
    struct mg_mgr mgr;                                
    mg_mgr_init(&mgr);                                      // Init manager
    mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL);  // Setup listener
    for (;;) mg_mgr_poll(&mgr, 1000);                       // Infinite event loop
}
