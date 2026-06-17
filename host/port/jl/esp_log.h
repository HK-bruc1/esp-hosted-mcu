/* host/port/jl/esp_log.h
 * Minimal ESP-IDF log compatibility for JL non-ESP-IDF builds.
 */

#ifndef __ESP_LOG_H__
#define __ESP_LOG_H__

#include "h_wrapper.h"

/* ESP-IDF log level constants */
#define ESP_LOG_NONE    0
#define ESP_LOG_ERROR   1
#define ESP_LOG_WARN    2
#define ESP_LOG_INFO    3
#define ESP_LOG_DEBUG   4
#define ESP_LOG_VERBOSE 5

#define ESP_LOGE(tag, fmt, ...) H_LOGE(tag, fmt, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) H_LOGW(tag, fmt, ##__VA_ARGS__)
#define ESP_LOGI(tag, fmt, ...) H_LOGI(tag, fmt, ##__VA_ARGS__)
#define ESP_LOGD(tag, fmt, ...) H_LOGD(tag, fmt, ##__VA_ARGS__)
#define ESP_LOGV(tag, fmt, ...) H_LOGV(tag, fmt, ##__VA_ARGS__)

#define ESP_LOG_BUFFER_HEX_LEVEL(tag, buf, len, lvl) ((void)0)
#define esp_log_level_set(tag, level) ((void)0)

#endif /* __ESP_LOG_H__ */
