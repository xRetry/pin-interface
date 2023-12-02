#include <stdio.h>
#include "pin_modes.h"
#include "driver/gpio.h"

esp_err_t init_disabled(pin_nr_t pin_nr) {
    //return gpio_set_direction(pin_nr, GPIO_MODE_DISABLE);
    return ESP_OK;
}

esp_err_t disabled(pin_nr_t pin_nr, double *val) {
    *val = 0;
    return ESP_OK;
}

esp_err_t digital_write(pin_nr_t pin_nr, double *val) {
    printf("digital_write: pin=%d, val=%f\n", pin_nr, *val);
    return gpio_set_level(pin_nr, *val);
}

esp_err_t init_digital_write(pin_nr_t pin_nr) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << pin_nr);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    return gpio_config(&io_conf);
}

esp_err_t digital_read(pin_nr_t pin_nr, double *val) {
    printf("set digital_read: pin=%d\n", pin_nr);
    *val = gpio_get_level(pin_nr);
    return ESP_OK;
}

esp_err_t init_digital_read(pin_nr_t pin_nr) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin_nr);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    return gpio_config(&io_conf);
}

const struct pin_mode_t PIN_MODES[] = {
    REGISTER_MODE("Disabled", PIN_DISABLED, init_disabled, disabled, ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9,10,18,19)),
    REGISTER_MODE("Digital Write", PIN_WRITE, init_digital_write, digital_write, ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9,10,18,19)),
    REGISTER_MODE("Digital Read", PIN_READ, init_digital_read, digital_read, ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9,10,18,19))
};

const int NUM_MODES = sizeof(PIN_MODES)/sizeof(struct pin_mode_t);
