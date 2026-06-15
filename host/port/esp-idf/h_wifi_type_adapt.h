/* host/port/esp-idf/h_wifi_type_adapt.h
 *
 * ESP-IDF port layer — bidirectional type adapters between
 * core-layer h_wifi_* types and ESP-IDF native wifi_* types.
 */

#ifndef H_WIFI_TYPE_ADAPT_H
#define H_WIFI_TYPE_ADAPT_H

#include "esp_wifi.h"
#include "esp_mac.h"
#include "h_types.h"
#include "h_wifi_types.h"

#ifndef H_WIFI_DUALBAND_SUPPORT
#define H_WIFI_DUALBAND_SUPPORT 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Struct adapters: portable -> native ── */
void h_wifi_adapt_init_config_to_native(const h_wifi_init_config_t *src, wifi_init_config_t *dst);
void h_wifi_adapt_config_to_native(const h_wifi_config_t *src, wifi_config_t *dst);
void h_wifi_adapt_scan_config_to_native(const h_wifi_scan_config_t *src, wifi_scan_config_t *dst);
void h_wifi_adapt_ap_record_to_native(const h_wifi_ap_record_t *src, wifi_ap_record_t *dst);
void h_wifi_adapt_sta_list_to_native(const h_wifi_sta_list_t *src, wifi_sta_list_t *dst);
void h_wifi_adapt_country_to_native(const h_wifi_country_t *src, wifi_country_t *dst);

/* ── Struct adapters: native -> portable ── */
void h_wifi_adapt_init_config_to_host(const wifi_init_config_t *src, h_wifi_init_config_t *dst);
void h_wifi_adapt_config_to_host(const wifi_config_t *src, h_wifi_config_t *dst);
void h_wifi_adapt_scan_config_to_host(const wifi_scan_config_t *src, h_wifi_scan_config_t *dst);
void h_wifi_adapt_ap_record_to_host(const wifi_ap_record_t *src, h_wifi_ap_record_t *dst);
void h_wifi_adapt_sta_list_to_host(const wifi_sta_list_t *src, h_wifi_sta_list_t *dst);
void h_wifi_adapt_country_to_host(const wifi_country_t *src, h_wifi_country_t *dst);

/* ── Enum converters ── */
wifi_interface_t h_wifi_adapt_iface_to_native(h_wifi_interface_t v);
h_wifi_interface_t h_wifi_adapt_iface_to_host(wifi_interface_t v);

wifi_mode_t h_wifi_adapt_mode_to_native(h_wifi_mode_t v);
h_wifi_mode_t h_wifi_adapt_mode_to_host(wifi_mode_t v);

wifi_ps_type_t h_wifi_adapt_ps_to_native(h_wifi_ps_type_t v);
h_wifi_ps_type_t h_wifi_adapt_ps_to_host(wifi_ps_type_t v);

wifi_bandwidth_t h_wifi_adapt_bw_to_native(h_wifi_bandwidth_t v);
h_wifi_bandwidth_t h_wifi_adapt_bw_to_host(wifi_bandwidth_t v);

wifi_auth_mode_t h_wifi_adapt_auth_to_native(h_wifi_auth_mode_t v);
h_wifi_auth_mode_t h_wifi_adapt_auth_to_host(wifi_auth_mode_t v);

wifi_cipher_type_t h_wifi_adapt_cipher_to_native(h_wifi_cipher_type_t v);
h_wifi_cipher_type_t h_wifi_adapt_cipher_to_host(wifi_cipher_type_t v);

wifi_second_chan_t h_wifi_adapt_second_chan_to_native(h_wifi_second_chan_t v);
h_wifi_second_chan_t h_wifi_adapt_second_chan_to_host(wifi_second_chan_t v);

wifi_phy_mode_t h_wifi_adapt_phymode_to_native(h_wifi_phy_mode_t v);
h_wifi_phy_mode_t h_wifi_adapt_phymode_to_host(wifi_phy_mode_t v);

wifi_band_t h_wifi_adapt_band_to_native(h_wifi_band_t v);
h_wifi_band_t h_wifi_adapt_band_to_host(wifi_band_t v);

#if H_WIFI_DUALBAND_SUPPORT
wifi_band_mode_t h_wifi_adapt_band_mode_to_native(h_wifi_band_mode_t v);
h_wifi_band_mode_t h_wifi_adapt_band_mode_to_host(wifi_band_mode_t v);
#endif

void h_wifi_adapt_scan_default_params_to_native(const h_wifi_scan_default_params_t *src, wifi_scan_default_params_t *dst);
void h_wifi_adapt_scan_default_params_to_host(const wifi_scan_default_params_t *src, h_wifi_scan_default_params_t *dst);

#if H_WIFI_DUALBAND_SUPPORT
void h_wifi_adapt_protocols_to_native(const h_wifi_protocols_t *src, wifi_protocols_t *dst);
void h_wifi_adapt_protocols_to_host(const wifi_protocols_t *src, h_wifi_protocols_t *dst);

void h_wifi_adapt_bandwidths_to_native(const h_wifi_bandwidths_t *src, wifi_bandwidths_t *dst);
void h_wifi_adapt_bandwidths_to_host(const wifi_bandwidths_t *src, h_wifi_bandwidths_t *dst);
#endif

esp_mac_type_t h_wifi_adapt_mac_type_to_native(h_mac_type_t v);
h_mac_type_t h_wifi_adapt_mac_type_to_host(esp_mac_type_t v);

wifi_vendor_ie_type_t h_wifi_adapt_vendor_ie_type_to_native(h_wifi_vendor_ie_type_t v);
h_wifi_vendor_ie_type_t h_wifi_adapt_vendor_ie_type_to_host(wifi_vendor_ie_type_t v);

wifi_vendor_ie_id_t h_wifi_adapt_vendor_ie_id_to_native(h_wifi_vendor_ie_id_t v);
h_wifi_vendor_ie_id_t h_wifi_adapt_vendor_ie_id_to_host(wifi_vendor_ie_id_t v);

#ifdef __cplusplus
}
#endif

#endif /* H_WIFI_TYPE_ADAPT_H */
