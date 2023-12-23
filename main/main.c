#include "webserver.h"
#include "wifi.h"
#include <stdio.h>
#include "pin_interface.h"
#include "driver/gpio.h"
#include "pin_modes.c"


int app_main() {
    wifi_init_sta();
    webserver_run();
    return 0;
}


