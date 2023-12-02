#ifndef _UTILS_H_
#define _UTILS_H_

#include "constants.h"

esp_err_t utils_fwrite_binary(const char *path, const void *content, const size_t length);
esp_err_t utils_fread_binary(const char *path, void *content, size_t length);
esp_err_t utils_string_to_long(const char *str, long *num);
esp_err_t utils_string_to_double(const char *str, double *num);
void utils_array_to_json(char *str, char *arr[], int arr_len, int str_len);

#endif
