/* host/port/jl/esp_system.h
 * JL shadow of ESP-IDF esp_system.h.
 *
 * Provides the minimal reset-reason type used by host/esp_hosted_event.h.
 */

#ifndef JL_ESP_SYSTEM_H
#define JL_ESP_SYSTEM_H

typedef enum {
    ESP_RST_UNKNOWN = 0,
    ESP_RST_POWERON,
    ESP_RST_EXT,
    ESP_RST_SW,
    ESP_RST_PANIC,
    ESP_RST_INT_WDT,
    ESP_RST_TASK_WDT,
    ESP_RST_WDT,
    ESP_RST_DEEPSLEEP,
    ESP_RST_BROWNOUT,
    ESP_RST_SDIO,
    ESP_RST_USB,
    ESP_RST_JTAG,
    ESP_RST_EFUSE,
    ESP_RST_PWR_GLITCH,
    ESP_RST_CPU_LOCKUP,
} esp_reset_reason_t;

#endif /* JL_ESP_SYSTEM_H */
