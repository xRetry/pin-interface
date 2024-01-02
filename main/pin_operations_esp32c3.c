#include <stdio.h>
#include "hal/adc_types.h"
#include "pin_interface.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "driver/i2c.h"

pi_err_t init_disabled(pi_pin_nr_t pin_nr) {
    //return gpio_set_direction(pin_nr, GPIO_MODE_DISABLE);
    return ESP_OK;
}

pi_err_t disabled(pi_pin_nr_t pin_nr, double *val) {
    *val = 0;
    return ESP_OK;
}

pi_err_t digital_write(pi_pin_nr_t pin_nr, double *val) {
    printf("digital_write: pin=%d, val=%f\n", pin_nr, *val);
    return gpio_set_level(pin_nr, *val);
}

pi_err_t init_digital_write(pi_pin_nr_t pin_nr) {
    printf("set_digital_write: pin=%d\n", pin_nr);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << pin_nr);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    return gpio_config(&io_conf);
}

pi_err_t digital_read(pi_pin_nr_t pin_nr, double *val) {
    printf("digital_write: pin=%d, val=%f\n", pin_nr, *val);
    *val = gpio_get_level(pin_nr);
    return ESP_OK;
}

pi_err_t init_digital_read(pi_pin_nr_t pin_nr) {
    printf("set digital_read: pin=%d\n", pin_nr);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin_nr);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    return gpio_config(&io_conf);
}

adc_oneshot_unit_handle_t adc_oneshot_handles[PI_NUM_PINS];

pi_err_t analog_read(pi_pin_nr_t pin_nr, double *val) {
    adc_unit_t adc_unit;
    adc_channel_t adc_channel;
    PI_OK_OR_RETURN(adc_oneshot_io_to_channel(pin_nr, &adc_unit, &adc_channel));

    adc_oneshot_read(adc_oneshot_handles[pin_nr], adc_channel, (int*) val);
    printf("analog_read: pin=%d, val=%f\n", pin_nr, *val);
    return ESP_OK;
}

pi_err_t init_analog_read(pi_pin_nr_t pin_nr) {
    printf("set analog_read: pin=%d\n", pin_nr);
    adc_unit_t adc_unit;
    adc_channel_t adc_channel;
    PI_OK_OR_RETURN(adc_oneshot_io_to_channel(pin_nr, &adc_unit, &adc_channel));

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = adc_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    PI_OK_OR_RETURN((adc_oneshot_new_unit(&init_config, &adc_oneshot_handles[pin_nr])));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    return adc_oneshot_config_channel(adc_oneshot_handles[pin_nr], adc_channel, &config);
}

pi_err_t i2c_read(pi_pin_nr_t address, double *val) {
    printf("analog_read: pin=%d\n", address);
    return i2c_master_read_from_device(0, address, (uint8_t*) val, sizeof(double), 10);
}

pi_err_t init_i2c_read(pi_pin_nr_t pin_nr) {
    printf("set i2c_read: pin=%d\n", pin_nr);
    int i2c_master_port = 0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = pin_nr,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = pin_nr+1,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
        .clk_flags = 0,
    };
    PI_OK_OR_RETURN(i2c_param_config(i2c_master_port, &conf));
    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

PI_REGISTER_OPS(
    PI_ADD_OP("Disabled", PI_DISABLED, init_disabled, disabled, PI_ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9,10,18,19)),
    PI_ADD_OP("Digital Write", PI_WRITE, init_digital_write, digital_write, PI_ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9,10,18,19)),
    PI_ADD_OP("Digital Read", PI_READ, init_digital_read, digital_read, PI_ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9,10,18,19)),
    PI_ADD_OP("Analog Read", PI_READ, init_analog_read, analog_read, PI_ALLOWED_PINS(0,1,2,3,4))
);
