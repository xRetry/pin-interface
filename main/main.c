#include "webserver.h"
#include "wifi.h"
#include <stdio.h>
#include "pin_interface.h"
#include "driver/gpio.h"
#include "pin_modes.c"
#include "nvs.h"
#include "nvs_flash.h"

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

int app_main() {

    nvs_init();
    wifi_init_sta();
    webserver_run();
    return 0;
}


