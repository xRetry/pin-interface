#include "utils.h"
#include "nvs_flash.h"

#define STORAGE_NAMESPACE "storage"

#define OK_OR_RETURN(fn) {\
    esp_err_t err = fn; \
    if (err != ESP_OK) { return err; } \
}

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
