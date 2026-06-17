/* host/port/jl/esp_wifi.h
 * JL shadow of ESP-IDF esp_wifi.h.
 *
 * Maps ESP-IDF Wi-Fi types to the portable h_wifi_types.h definitions.
 * This allows host/api/src/esp_wifi_weak.c and esp_hosted_api.c to compile
 * unchanged on non-ESP-IDF platforms.
 */

#ifndef JL_ESP_WIFI_H
#define JL_ESP_WIFI_H

#include "h_wifi_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Type aliases to ESP-IDF names used by esp_hosted_api_priv.h */
typedef h_wifi_init_config_t   wifi_init_config_t;
typedef h_wifi_config_t        wifi_config_t;
typedef h_wifi_scan_config_t   wifi_scan_config_t;
typedef h_wifi_ap_record_t     wifi_ap_record_t;
typedef h_wifi_sta_list_t      wifi_sta_list_t;
typedef h_wifi_country_t       wifi_country_t;
typedef h_wifi_interface_t     wifi_interface_t;
typedef h_wifi_mode_t          wifi_mode_t;
typedef h_wifi_ps_type_t       wifi_ps_type_t;
typedef h_wifi_bandwidth_t     wifi_bandwidth_t;
typedef h_wifi_second_chan_t   wifi_second_chan_t;
typedef h_wifi_phy_mode_t      wifi_phy_mode_t;
typedef h_wifi_band_t          wifi_band_t;
typedef h_wifi_band_mode_t     wifi_band_mode_t;
typedef h_wifi_auth_mode_t     wifi_auth_mode_t;
typedef h_wifi_storage_t       wifi_storage_t;
typedef h_wifi_cipher_type_t   wifi_cipher_type_t;
typedef h_wifi_scan_default_params_t wifi_scan_default_params_t;
typedef h_wifi_protocols_t     wifi_protocols_t;
typedef h_wifi_bandwidths_t    wifi_bandwidths_t;
typedef h_wifi_twt_config_t    wifi_twt_config_t;
typedef h_wifi_twt_setup_config_t wifi_twt_setup_config_t;

#define WIFI_IF_STA  H_WIFI_IF_STA
#define WIFI_IF_AP   H_WIFI_IF_AP

/* Standard ESP-IDF mode constants */
#define WIFI_MODE_NULL   H_WIFI_MODE_NULL
#define WIFI_MODE_STA    H_WIFI_MODE_STA
#define WIFI_MODE_AP     H_WIFI_MODE_AP
#define WIFI_MODE_APSTA  H_WIFI_MODE_APSTA

#ifdef __cplusplus
}
#endif

#endif /* JL_ESP_WIFI_H */
