#include <stdio.h>
#include "pin_interface.h"

#define MQTT 1
//#define WEBSERVER 0

#ifdef WEBSERVER
#include "webserver.h"
#elif MQTT
#include "mqtt.h"
#endif
//#undef PI_ARCH_ESP32
//#define PI_ARCH_LINUX 1

#ifdef PI_ARCH_ESP32
    #include "wifi.h"
    #include "nvs.h"
    #include "nvs_flash.h"
    #include "pin_operations_esp32c3.c"
#elif PI_ARCH_LINUX
    #include "pin_operations_linux.c"
#endif

#ifdef PI_ARCH_ESP32
void nvs_init() {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}
#endif

#ifdef PI_ARCH_ESP32
int app_main() {
    nvs_init();
    wifi_init_sta();
#elif PI_ARCH_LINUX
int main() {
#endif

    pi_init();
#ifdef WEBSERVER
    webserver_run();
#elif MQTT
    mqtt_client_run();
#endif
    return 0;
}


