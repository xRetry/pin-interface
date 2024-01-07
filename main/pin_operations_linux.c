#include <stdio.h>
#include "pin_interface.h"

pi_err_t init_disabled(pi_pin_op_nr_t pin_op_nr) {
    return 0;
}

pi_err_t disabled(pi_pin_nr_t pin_nr, double *val) {
    *val = 0;
    return 0;
}

pi_err_t digital_write(pi_pin_nr_t pin_nr, double *val) {
    printf("digital_write: pin=%d, val=%f\n", pin_nr, *val);
    return 0;
}

pi_err_t init_digital_write(pi_pin_nr_t pin_nr) {
    printf("init digital_write: pin=%d\n", pin_nr);
    return 0;
}

pi_err_t digital_read(pi_pin_nr_t pin_nr, double *val) {
    printf("digital_read: pin=%d\n", pin_nr);
    *val = pin_nr;
    return 0;
}

pi_err_t init_digital_read(pi_pin_nr_t pin_nr) {
    printf("init digital_read: pin=%d\n", pin_nr);
    return 0;
}

PI_REGISTER_OPS(
    PI_ADD_OP("Disabled", PI_DISABLED, init_disabled, disabled, PI_ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9)),
    PI_ADD_OP("Digital Write", PI_WRITE, init_digital_write, digital_write, PI_ALLOWED_PINS(1, 3)),
    PI_ADD_OP("Digital Read", PI_READ, init_digital_read, digital_read, PI_ALLOWED_PINS(4, 5))
);

