#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "board.h"
#include <stdbool.h>
//#include "utils.h"
#include "pin_interface.h"


struct interface_t {
    struct state_t {
        pi_pin_mode_nr_t mode_nrs[NUM_PINS];
    } state;

    pi_err_t (*rw_functions[NUM_PINS])(pi_pin_nr_t, double*);
    enum pi_pin_dir_t directions[NUM_PINS];
    pi_pin_mode_nr_t allowed_modes[NUM_PINS];
} interface;

void allowed_init(void) {
    for (int i=0; i<NUM_PINS; ++i) {
        interface.allowed_modes[i] = 0;
    }

    for (int i=0; i<PI_NUM_MODES; ++i) {
        struct pi_pin_mode_t pin_mode = PI_PIN_MODES[i];
        for (int j=0; j<pin_mode.pins_allowed_size; ++j) {
            interface.allowed_modes[j] |= 1<<i;
        }
    }
}

void state_init(void) {
    //esp_err_t err = utils_fread_binary("modes", interface.state.mode_nrs, sizeof(interface.state.mode_nrs));
    if (err != 0) {
        for (int i=0; i<NUM_PINS; ++i) {
            interface.state.mode_nrs[i] = 0;
        }
    }
}

void modes_init(void) {
    for (int i=0; i<NUM_PINS; ++i) {
        struct pi_pin_mode_t pin_mode = PI_PIN_MODES[interface.state.mode_nrs[i]];
        pin_mode.fn_init(i);
        interface.rw_functions[i] = pin_mode.fn_rw;
        interface.directions[i] = pin_mode.direction;
    }
}

void pi_init(void) {
    allowed_init();
    state_init();
    modes_init();
}

pi_err_t pi_exec_pin_op(pi_pin_nr_t pin_nr, double *val) {
    if (pin_nr >= NUM_PINS) { 
        return 1; 
    }

    interface.rw_functions[pin_nr](pin_nr, val);
    return 0;
}

void pi_init_pin_modes(const pi_pin_mode_nr_t new_modes_nrs[NUM_PINS]) {
    for (int i=0; i<NUM_PINS; ++i) {
        struct pin_mode_t pin_mode = PIN_MODES[new_modes_nrs[i]];

        bool is_mode_allowed = false;
        for (int j=0; j<pin_mode.pins_allowed_size; ++j) {
            if (pin_mode.pins_allowed[j] == i) {
                is_mode_allowed = true;
                break;
            }
        }

        if (!is_mode_allowed || PI_ERR(pin_mode.fn_init(i))) { 
            // TODO(marco): Save errors
            continue; 
        };
        interface.rw_functions[i] = pin_mode.fn_rw;
        interface.directions[i] = pin_mode.direction;
        interface.state.mode_nrs[i] = new_modes_nrs[i];
    }
}
