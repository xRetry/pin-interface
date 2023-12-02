#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nvs_flash.h"

#include "utils.h"

#define STORAGE_NAMESPACE "storage"

esp_err_t utils_fwrite_binary(const char *path, const void *content, const size_t length) {
    nvs_handle_t nvs_handle;

    OK_OR_RETURN(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle));
    OK_OR_RETURN(nvs_set_blob(nvs_handle, path, content, length));
    OK_OR_RETURN(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);

    return ESP_OK;
}

esp_err_t utils_fread_binary(const char *path, void *content, size_t length) {
    nvs_handle_t nvs_handle;

    OK_OR_RETURN(nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle));
    OK_OR_RETURN(nvs_get_blob(nvs_handle, path, content, &length));
    nvs_close(nvs_handle);

    return ESP_OK;
}

esp_err_t utils_string_to_long(const char *str, long *num) {
    errno = 0;
    char *endptr = NULL;
    *num = strtol(str, &endptr, 10);
    if (errno == 0 && str && !*endptr) {
        return 0;
    } else if (errno == 0 && str && *endptr != 0) {
        return 0;
    }
    return 1;
}

esp_err_t utils_string_to_double(const char *str, double *num) {
    errno = 0;
    char *endptr = NULL;
    *num = strtod(str, &endptr);
    if (errno == 0 && str && !*endptr) {
        return 0;
    } else if (errno == 0 && str && *endptr != 0) {
        return 0;
    }
    return 1;
}

void utils_array_to_json(char *str, char *arr[], int arr_len, int str_len) {
    strcat(str, "[");
    for (int i=0; i<arr_len; ++i) {
        strcat(str, arr[i]);

        char delimiter[] = ",";
        if (i == arr_len-1) { delimiter[0] = '\0'; }
        strcat(str, delimiter);
    }
    strcat(str, "]");
}
