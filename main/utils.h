#ifndef _UTILS_H_
#define _UTILS_H_

#include "esp_err.h"

esp_err_t utils_fwrite_binary(const char *path, const void *content, const size_t length);
esp_err_t utils_fread_binary(const char *path, void *content, size_t length);

#endif
