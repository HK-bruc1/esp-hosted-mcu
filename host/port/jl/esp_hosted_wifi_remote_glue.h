/* host/port/jl/esp_hosted_wifi_remote_glue.h
 * JL shadow of host/api/include/esp_hosted_wifi_remote_glue.h.
 *
 * Keeps the public channel-glue API intact while using the JL shadows of
 * esp_wifi.h / esp_wifi_remote.h instead of ESP-IDF headers.
 */

#ifndef __ESP_HOSTED_WIFI_REMOTE_GLUE_JL_H__
#define __ESP_HOSTED_WIFI_REMOTE_GLUE_JL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_hosted_interface.h"
#include "esp_wifi_remote.h"
#include "esp_wifi.h"

struct esp_remote_channel_config {
    esp_hosted_if_type_t if_type;
    bool secure;
};

typedef struct esp_remote_channel_config * esp_remote_channel_config_t;

#define ESP_HOSTED_CHANNEL_CONFIG_DEFAULT()  { \
    .secure = true,                            \
}

typedef esp_err_t (*esp_remote_channel_rx_fn_t)(void *h, void *buffer,
        void *buff_to_free, size_t len);
typedef esp_err_t (*esp_remote_channel_tx_fn_t)(void *h, void *buffer, size_t len);

esp_remote_channel_t esp_hosted_add_channel(esp_remote_channel_config_t config,
        esp_remote_channel_tx_fn_t *tx, const esp_remote_channel_rx_fn_t rx);
esp_err_t esp_hosted_remove_channel(esp_remote_channel_t channel);

#ifdef __cplusplus
}
#endif

#endif /* __ESP_HOSTED_WIFI_REMOTE_GLUE_JL_H__ */
