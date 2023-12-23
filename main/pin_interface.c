#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "pin_interface.h"

#define PI_ARCH_LINUX 0
#define PI_ARCH_ESP32 1

#if !defined(PI_ARCH)
    #if defined(__unix__) || defined(__APPLE__)
        #define PI_ARCH PI_ARCH_LINUX
    #elif defined(ESP_PLATFORM)
        #define PI_ARCH PI_ARCH_ESP32
    #endif
#endif

#ifdef PI_ARCH_ESP32
    #include "nvs_flash.h"
    #define STORAGE_NAMESPACE "pi_storage"
#endif

struct pi_state_t pi_state;

pi_err_t fwrite_binary(const char *path, const void *content, const size_t size, const int n) {
#ifdef PI_ARCH_ESP32
    nvs_handle_t nvs_handle;

    PI_OK_OR_RETURN(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle));
    PI_OK_OR_RETURN(nvs_set_blob(nvs_handle, path, content, size*n));
    PI_OK_OR_RETURN(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);

    return ESP_OK;
#else
    FILE *fptr = fopen(path, "wb");
    if (fptr == NULL) {
        printf("Error opening file `%s`\n", path);
        return 1;
    }

    fwrite(content, size, n, fptr);
    fclose(fptr);
    return 0;
#endif
}

pi_err_t fread_binary(const char *path, void *content, const size_t size, const int n) {
#ifdef PI_ARCH_ESP32
    nvs_handle_t nvs_handle;
    size_t length = size*n;

    PI_OK_OR_RETURN(nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle));
    PI_OK_OR_RETURN(nvs_get_blob(nvs_handle, path, content, &length));
    nvs_close(nvs_handle);

    return ESP_OK;
#else
    FILE *fptr = fopen(path, "rb");
    if (fptr == NULL) {
        printf("Error opening file `%s`\n", path);
        return 1;
    }

    int ret = fread(content, size, n, fptr);
    if (ret != PI_NUM_PINS) {
        printf("Error reading file `%s`: %d\n", path, ret);
        return 1;
    }
    fclose(fptr);
    return 0;
#endif
}

void allowed_init(void) {
    for (int i=0; i<PI_NUM_PINS; ++i) {
        pi_state.allowed_op_nrs[i] = 0;
    }

    for (int i=0; i<PI_NUM_OPS; ++i) {
        struct pi_pin_op_t pin_op = PI_PIN_OPS[i];
        for (int j=0; j<pin_op.pins_allowed_size; ++j) {
            pi_state.allowed_op_nrs[j] |= 1<<i;
        }
    }
}

void ops_init(void) {
    pi_err_t err = fread_binary("pi_active_ops", pi_state.active_op_nrs, sizeof(pi_pin_nr_t), PI_NUM_PINS);
    if (err != 0) {
        for (int i=0; i<PI_NUM_PINS; ++i) {
            pi_state.active_op_nrs[i] = 0;
        }
    }

    for (int i=0; i<PI_NUM_PINS; ++i) {
        struct pi_pin_op_t pin_op = PI_PIN_OPS[pi_state.active_op_nrs[i]];
        pin_op.fn_init(i);
        pi_state.rw_functions[i] = pin_op.fn_rw;
        pi_state.directions[i] = pin_op.direction;
    }
}

void pi_init(void) {
    allowed_init();
    ops_init();
}

pi_err_t pi_exec_pin_op(pi_pin_nr_t pin_nr, double *val) {
    if (pin_nr >= PI_NUM_PINS) { 
        return 1; 
    }

    pi_state.rw_functions[pin_nr](pin_nr, val);
    return 0;
}

pi_err_t pi_init_pin_op(const pi_pin_nr_t pin_nr, const pi_pin_op_nr_t new_op_nr) {
    struct pi_pin_op_t pin_op = PI_PIN_OPS[new_op_nr];

    bool is_op_allowed = false;
    for (int j=0; j<pin_op.pins_allowed_size; ++j) {
        if (pin_op.pins_allowed[j] == pin_nr) {
            is_op_allowed = true;
            break;
        }
    }

    if (!is_op_allowed || PI_IS_ERR(pin_op.fn_init(pin_nr))) { 
        // TODO(marco): Save errors
        return 1;
    };
    pi_state.rw_functions[pin_nr] = pin_op.fn_rw;
    pi_state.directions[pin_nr] = pin_op.direction;
    pi_state.active_op_nrs[pin_nr] = new_op_nr;

    fwrite_binary("pi_active_ops", pi_state.active_op_nrs, sizeof(pi_pin_nr_t), PI_NUM_PINS);
    return 0;
}
