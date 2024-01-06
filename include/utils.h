#ifndef _UTILS_H_
#define _UTILS_H_

#include "constants.h"
#include "pin_interface.h"

pi_err_t utils_string_to_long(const char *str, long *num);
pi_err_t utils_string_to_double(const char *str, double *num);
void utils_array_to_json(char *str, char *arr[], int arr_len, int str_len);

#endif
