/* host/port/jl/h_wifi.c
 * JL Wi-Fi type conversion contract — Phase 1 pass-through.
 */

#include "h_port_contract.h"
#include "h_wifi_types.h"
#include "rpc_slave_if.h"
#include <string.h>

static void jl_init_config_to_req(const h_wifi_init_config_t *src, void *req)
{
    if (src && req) {
        memcpy(req, src, sizeof(*src));
    }
}

static void jl_scan_config_to_req(const h_wifi_scan_config_t *src, void *req)
{
    if (src && req) {
        memcpy(req, src, sizeof(*src));
    }
}

static void jl_country_to_req(const h_wifi_country_t *src, void *req)
{
    if (src && req) {
        memcpy(req, src, sizeof(*src));
    }
}

static void jl_ap_record_from_resp(const void *resp, h_wifi_ap_record_t *dst)
{
    if (resp && dst) {
        memcpy(dst, resp, sizeof(*dst));
    }
}

static void jl_ap_record_from_resp_list(const void *resp, h_wifi_ap_record_t *dst)
{
    jl_ap_record_from_resp(resp, dst);
}

static void jl_country_from_resp(const void *resp, h_wifi_country_t *dst)
{
    if (resp && dst) {
        memcpy(dst, resp, sizeof(*dst));
    }
}

static void jl_sta_list_from_resp(const void *resp, h_wifi_sta_list_t *dst)
{
    if (resp && dst) {
        memcpy(dst, resp, sizeof(*dst));
    }
}

static uint8_t jl_iface_to_native(h_wifi_interface_t v)
{
    return (uint8_t)v;
}

static uint8_t jl_mode_to_native(h_wifi_mode_t v)
{
    return (uint8_t)v;
}

static uint8_t jl_ps_to_native(h_wifi_ps_type_t v)
{
    return (uint8_t)v;
}

static uint8_t jl_bw_to_native(h_wifi_bandwidth_t v)
{
    return (uint8_t)v;
}

static h_wifi_interface_t jl_iface_to_host(uint8_t v)
{
    return (h_wifi_interface_t)v;
}

static h_wifi_mode_t jl_mode_to_host(uint8_t v)
{
    return (h_wifi_mode_t)v;
}

static h_wifi_ps_type_t jl_ps_to_host(uint8_t v)
{
    return (h_wifi_ps_type_t)v;
}

static h_wifi_bandwidth_t jl_bw_to_host(uint8_t v)
{
    return (h_wifi_bandwidth_t)v;
}

const h_wifi_contract_t g_h_wifi = {
    .init_config_to_req      = jl_init_config_to_req,
    .scan_config_to_req      = jl_scan_config_to_req,
    .country_to_req          = jl_country_to_req,
    .ap_record_from_resp     = jl_ap_record_from_resp,
    .ap_record_from_resp_list = jl_ap_record_from_resp_list,
    .country_from_resp       = jl_country_from_resp,
    .sta_list_from_resp      = jl_sta_list_from_resp,
    .iface_to_native         = jl_iface_to_native,
    .mode_to_native          = jl_mode_to_native,
    .ps_to_native            = jl_ps_to_native,
    .bw_to_native            = jl_bw_to_native,
    .iface_to_host           = jl_iface_to_host,
    .mode_to_host            = jl_mode_to_host,
    .ps_to_host              = jl_ps_to_host,
    .bw_to_host              = jl_bw_to_host,
};

/* Compile-time checks that the portable h_wifi_* types match the storage
 * layout used in the RPC ctrl_cmd_t union.  These catch accidental struct
 * drift in h_wifi_types.h. */
_Static_assert(sizeof(h_wifi_init_config_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_init_config),
               "h_wifi_init_config_t size mismatch with ctrl_cmd_t union");

_Static_assert(sizeof(h_wifi_config_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_config.u),
               "h_wifi_config_t size mismatch with ctrl_cmd_t union");

_Static_assert(sizeof(h_wifi_scan_config_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_scan_config.cfg),
               "h_wifi_scan_config_t size mismatch with ctrl_cmd_t union");

_Static_assert(sizeof(h_wifi_ap_record_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_ap_record),
               "h_wifi_ap_record_t size mismatch with ctrl_cmd_t union");

_Static_assert(sizeof(h_wifi_country_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_country),
               "h_wifi_country_t size mismatch with ctrl_cmd_t union");

_Static_assert(sizeof(h_wifi_sta_list_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_ap_sta_list),
               "h_wifi_sta_list_t size mismatch with ctrl_cmd_t union");

/* Enum-value checks: the slave firmware (ESP-IDF) expects the raw values of
 * interface, mode, power-save, and bandwidth enums.  These asserts catch
 * semantic drift that size checks cannot detect. */
_Static_assert(H_WIFI_IF_STA == 0 && H_WIFI_IF_AP == 1,
               "h_wifi_interface_t values must match ESP-IDF wifi_interface_t");

_Static_assert(H_WIFI_MODE_NULL == 0 && H_WIFI_MODE_STA == 1 &&
               H_WIFI_MODE_AP == 2 && H_WIFI_MODE_APSTA == 3,
               "h_wifi_mode_t values must match ESP-IDF wifi_mode_t");

_Static_assert(H_WIFI_PS_NONE == 0 && H_WIFI_PS_MIN_MODEM == 1 &&
               H_WIFI_PS_MAX_MODEM == 2,
               "h_wifi_ps_type_t values must match ESP-IDF wifi_ps_type_t");

_Static_assert(H_WIFI_BW_HT20 == 1 && H_WIFI_BW_HT40 == 2,
               "h_wifi_bandwidth_t values must match ESP-IDF wifi_bandwidth_t");
