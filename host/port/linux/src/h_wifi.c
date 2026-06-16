/* host/port/linux/src/h_wifi.c
 * Linux mock Wi-Fi conversion contract stub vtable for mock builds.
 *
 * IMPORTANT: These are compile-only placeholders. They do NOT perform
 * real field-level conversion. The returned values are arbitrary defaults
 * (e.g. H_WIFI_IF_STA, H_WIFI_MODE_NULL) and MUST NOT be relied upon in
 * tests that verify behavioral correctness of Wi-Fi type conversion.
 * Tests should either:
 *   (a) exercise the ESP-IDF port's real g_h_wifi implementation, or
 *   (b) install a test-specific fixture that overrides g_h_wifi.
 */

#include "h_port_contract.h"
#include "h_wifi_types.h"

static void stub_init_config_to_req(const h_wifi_init_config_t *src, void *req_wifi_init_config)
{ (void)src; (void)req_wifi_init_config; }

static void stub_scan_config_to_req(const h_wifi_scan_config_t *src, void *req_wifi_scan_config_cfg)
{ (void)src; (void)req_wifi_scan_config_cfg; }

static void stub_country_to_req(const h_wifi_country_t *src, void *req_wifi_country)
{ (void)src; (void)req_wifi_country; }

static void stub_ap_record_from_resp(const void *resp_wifi_ap_record, h_wifi_ap_record_t *dst)
{ (void)resp_wifi_ap_record; (void)dst; }

static void stub_ap_record_from_resp_list(const void *resp_wifi_scan_ap_list_out_list, h_wifi_ap_record_t *dst)
{ (void)resp_wifi_scan_ap_list_out_list; (void)dst; }

static void stub_country_from_resp(const void *resp_wifi_country, h_wifi_country_t *dst)
{ (void)resp_wifi_country; (void)dst; }

static void stub_sta_list_from_resp(const void *resp_wifi_ap_sta_list, h_wifi_sta_list_t *dst)
{ (void)resp_wifi_ap_sta_list; (void)dst; }

static uint8_t stub_iface_to_native(h_wifi_interface_t v)
{ (void)v; return 0; }

static uint8_t stub_mode_to_native(h_wifi_mode_t v)
{ (void)v; return 0; }

static uint8_t stub_ps_to_native(h_wifi_ps_type_t v)
{ (void)v; return 0; }

static uint8_t stub_bw_to_native(h_wifi_bandwidth_t v)
{ (void)v; return 0; }

static h_wifi_interface_t stub_iface_to_host(uint8_t v)
{ (void)v; return H_WIFI_IF_STA; }

static h_wifi_mode_t stub_mode_to_host(uint8_t v)
{ (void)v; return H_WIFI_MODE_NULL; }

static h_wifi_ps_type_t stub_ps_to_host(uint8_t v)
{ (void)v; return H_WIFI_PS_NONE; }

static h_wifi_bandwidth_t stub_bw_to_host(uint8_t v)
{ (void)v; return H_WIFI_BW_HT20; }

const h_wifi_contract_t g_h_wifi = {
    .init_config_to_req       = stub_init_config_to_req,
    .scan_config_to_req       = stub_scan_config_to_req,
    .country_to_req           = stub_country_to_req,
    .ap_record_from_resp      = stub_ap_record_from_resp,
    .ap_record_from_resp_list = stub_ap_record_from_resp_list,
    .country_from_resp        = stub_country_from_resp,
    .sta_list_from_resp       = stub_sta_list_from_resp,
    .iface_to_native          = stub_iface_to_native,
    .mode_to_native           = stub_mode_to_native,
    .ps_to_native             = stub_ps_to_native,
    .bw_to_native             = stub_bw_to_native,
    .iface_to_host            = stub_iface_to_host,
    .mode_to_host             = stub_mode_to_host,
    .ps_to_host               = stub_ps_to_host,
    .bw_to_host               = stub_bw_to_host,
};
