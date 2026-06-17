/* host/port/jl/esp_check.h
 * JL shadow of ESP-IDF esp_check.h.
 *
 * Provides the ESP_RETURN_ON_ERROR / ESP_GOTO_ON_ERROR style macros that
 * host/api/src/esp_hosted_api.c expects.
 */

#ifndef JL_ESP_CHECK_H
#define JL_ESP_CHECK_H

#include "esp_err.h"
#include "esp_log.h"

#define ESP_RETURN_ON_ERROR(x, tag, ...) do { \
        esp_err_t __err_rc = (x); \
        if (__err_rc != ESP_OK) { \
            ESP_LOGE(tag, __VA_ARGS__); \
            return __err_rc; \
        } \
    } while(0)

#define ESP_GOTO_ON_ERROR(x, goto_tag, tag, ...) do { \
        esp_err_t __err_rc = (x); \
        if (__err_rc != ESP_OK) { \
            ESP_LOGE(tag, __VA_ARGS__); \
            goto goto_tag; \
        } \
    } while(0)

#define ESP_RETURN_ON_FALSE(a, err_code, tag, ...) do { \
        if (!(a)) { \
            ESP_LOGE(tag, __VA_ARGS__); \
            return err_code; \
        } \
    } while(0)

#define ESP_GOTO_ON_FALSE(a, err_code, goto_tag, tag, ...) do { \
        if (!(a)) { \
            ESP_LOGE(tag, __VA_ARGS__); \
            goto goto_tag; \
        } \
    } while(0)

#endif /* JL_ESP_CHECK_H */
