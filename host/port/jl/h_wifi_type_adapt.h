/* host/port/jl/h_wifi_type_adapt.h
 * JL no-op type adapter.
 *
 * On JL, ESP-IDF wifi_* types are aliased directly to h_wifi_* types via
 * esp_wifi.h, so no field-by-field conversion is needed.  The adapter
 * macros below simply copy/assign the values.
 */

#ifndef H_WIFI_TYPE_ADAPT_JL_H
#define H_WIFI_TYPE_ADAPT_JL_H

#include "esp_wifi.h"
#include "h_wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void h_wifi_adapt_init_config_to_native(const h_wifi_init_config_t *src, wifi_init_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_config_to_native(const h_wifi_config_t *src, wifi_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_scan_config_to_native(const h_wifi_scan_config_t *src, wifi_scan_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_ap_record_to_native(const h_wifi_ap_record_t *src, wifi_ap_record_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_sta_list_to_native(const h_wifi_sta_list_t *src, wifi_sta_list_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_country_to_native(const h_wifi_country_t *src, wifi_country_t *dst)
{
    if (src && dst) *dst = *src;
}

static inline void h_wifi_adapt_init_config_to_host(const wifi_init_config_t *src, h_wifi_init_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_config_to_host(const wifi_config_t *src, h_wifi_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_scan_config_to_host(const wifi_scan_config_t *src, h_wifi_scan_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_ap_record_to_host(const wifi_ap_record_t *src, h_wifi_ap_record_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_sta_list_to_host(const wifi_sta_list_t *src, h_wifi_sta_list_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_country_to_host(const wifi_country_t *src, h_wifi_country_t *dst)
{
    if (src && dst) *dst = *src;
}

static inline wifi_interface_t h_wifi_adapt_iface_to_native(h_wifi_interface_t v) { return (wifi_interface_t)v; }
static inline h_wifi_interface_t h_wifi_adapt_iface_to_host(wifi_interface_t v) { return (h_wifi_interface_t)v; }

static inline wifi_mode_t h_wifi_adapt_mode_to_native(h_wifi_mode_t v) { return (wifi_mode_t)v; }
static inline h_wifi_mode_t h_wifi_adapt_mode_to_host(wifi_mode_t v) { return (h_wifi_mode_t)v; }

static inline wifi_ps_type_t h_wifi_adapt_ps_to_native(h_wifi_ps_type_t v) { return (wifi_ps_type_t)v; }
static inline h_wifi_ps_type_t h_wifi_adapt_ps_to_host(wifi_ps_type_t v) { return (h_wifi_ps_type_t)v; }

static inline wifi_bandwidth_t h_wifi_adapt_bw_to_native(h_wifi_bandwidth_t v) { return (wifi_bandwidth_t)v; }
static inline h_wifi_bandwidth_t h_wifi_adapt_bw_to_host(wifi_bandwidth_t v) { return (h_wifi_bandwidth_t)v; }

static inline wifi_auth_mode_t h_wifi_adapt_auth_to_native(h_wifi_auth_mode_t v) { return (wifi_auth_mode_t)v; }
static inline h_wifi_auth_mode_t h_wifi_adapt_auth_to_host(wifi_auth_mode_t v) { return (h_wifi_auth_mode_t)v; }

static inline wifi_second_chan_t h_wifi_adapt_second_chan_to_native(h_wifi_second_chan_t v) { return (wifi_second_chan_t)v; }
static inline h_wifi_second_chan_t h_wifi_adapt_second_chan_to_host(wifi_second_chan_t v) { return (h_wifi_second_chan_t)v; }

static inline wifi_phy_mode_t h_wifi_adapt_phymode_to_native(h_wifi_phy_mode_t v) { return (wifi_phy_mode_t)v; }
static inline h_wifi_phy_mode_t h_wifi_adapt_phymode_to_host(wifi_phy_mode_t v) { return (h_wifi_phy_mode_t)v; }

static inline wifi_band_t h_wifi_adapt_band_to_native(h_wifi_band_t v) { return (wifi_band_t)v; }
static inline h_wifi_band_t h_wifi_adapt_band_to_host(wifi_band_t v) { return (h_wifi_band_t)v; }

static inline wifi_band_mode_t h_wifi_adapt_band_mode_to_native(h_wifi_band_mode_t v) { return (wifi_band_mode_t)v; }
static inline h_wifi_band_mode_t h_wifi_adapt_band_mode_to_host(wifi_band_mode_t v) { return (h_wifi_band_mode_t)v; }

static inline void h_wifi_adapt_scan_default_params_to_native(const h_wifi_scan_default_params_t *src, wifi_scan_default_params_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_scan_default_params_to_host(const wifi_scan_default_params_t *src, h_wifi_scan_default_params_t *dst)
{
    if (src && dst) *dst = *src;
}

static inline void h_wifi_adapt_protocols_to_native(const h_wifi_protocols_t *src, wifi_protocols_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_protocols_to_host(const wifi_protocols_t *src, h_wifi_protocols_t *dst)
{
    if (src && dst) *dst = *src;
}

static inline void h_wifi_adapt_bandwidths_to_native(const h_wifi_bandwidths_t *src, wifi_bandwidths_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_bandwidths_to_host(const wifi_bandwidths_t *src, h_wifi_bandwidths_t *dst)
{
    if (src && dst) *dst = *src;
}

static inline void h_wifi_adapt_twt_config_to_native(const h_wifi_twt_config_t *src, wifi_twt_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_twt_config_to_host(const wifi_twt_config_t *src, h_wifi_twt_config_t *dst)
{
    if (src && dst) *dst = *src;
}

static inline void h_wifi_adapt_twt_setup_config_to_native(const h_wifi_twt_setup_config_t *src, wifi_twt_setup_config_t *dst)
{
    if (src && dst) *dst = *src;
}
static inline void h_wifi_adapt_twt_setup_config_to_host(const wifi_twt_setup_config_t *src, h_wifi_twt_setup_config_t *dst)
{
    if (src && dst) *dst = *src;
}

#ifdef __cplusplus
}
#endif

#endif /* H_WIFI_TYPE_ADAPT_JL_H */
