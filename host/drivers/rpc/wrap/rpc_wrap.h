/*
 * SPDX-FileCopyrightText: 2015-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** prevent recursive inclusion **/
#ifndef __RPC_WRAP_H__
#define __RPC_WRAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Includes **/
#include "h_wifi_types.h"
#include "h_dpp_types.h"
#include "h_eap_types.h"
#include "h_port_config.h"

#include "esp_hosted_api_types.h"
#include "esp_hosted_misc_types.h"

#if H_GPIO_EXPANDER_SUPPORT
#include "esp_hosted_cp_gpio.h"
#endif
#if H_EXT_COEX_SUPPORT
#include "esp_hosted_cp_ext_coex.h"
#endif
#if H_HOST_OT_ENABLE
#include "esp_hosted_openthread.h"
#endif
/** Exported variables **/

/** Inline functions **/

/** Exported Functions **/
h_err_t rpc_init(void);
h_err_t rpc_start(void);
h_err_t rpc_stop(void);
h_err_t rpc_deinit(void);
h_err_t rpc_unregister_event_callbacks(void);
h_err_t rpc_register_event_callbacks(void);

h_err_t rpc_wifi_init(const h_wifi_init_config_t *arg);
h_err_t rpc_wifi_deinit(void);
h_err_t rpc_wifi_set_mode(h_wifi_mode_t mode);
h_err_t rpc_wifi_get_mode(h_wifi_mode_t* mode);
h_err_t rpc_wifi_start(void);
h_err_t rpc_wifi_stop(void);
h_err_t rpc_wifi_connect(void);
h_err_t rpc_wifi_disconnect(void);
h_err_t rpc_wifi_set_config(h_wifi_interface_t interface, h_wifi_config_t *conf);
h_err_t rpc_wifi_get_config(h_wifi_interface_t interface, h_wifi_config_t *conf);
h_err_t rpc_wifi_get_mac(h_wifi_interface_t mode, uint8_t mac[6]);
h_err_t rpc_wifi_set_mac(h_wifi_interface_t mode, const uint8_t mac[6]);
h_err_t rpc_wifi_set_scan_parameters(const h_wifi_scan_default_params_t *config);
h_err_t rpc_wifi_get_scan_parameters(h_wifi_scan_default_params_t *config);

h_err_t rpc_wifi_scan_start(const h_wifi_scan_config_t *config, bool block);
h_err_t rpc_wifi_scan_stop(void);
h_err_t rpc_wifi_scan_get_ap_num(uint16_t *number);
h_err_t rpc_wifi_scan_get_ap_record(h_wifi_ap_record_t *ap_record);
h_err_t rpc_wifi_scan_get_ap_records(uint16_t *number, h_wifi_ap_record_t *ap_records);
h_err_t rpc_wifi_clear_ap_list(void);
h_err_t rpc_wifi_restore(void);
h_err_t rpc_wifi_clear_fast_connect(void);
h_err_t rpc_wifi_deauth_sta(uint16_t aid);
h_err_t rpc_wifi_sta_get_ap_info(h_wifi_ap_record_t *ap_info);
h_err_t rpc_wifi_set_ps(h_wifi_ps_type_t type);
h_err_t rpc_wifi_get_ps(h_wifi_ps_type_t *type);
h_err_t rpc_wifi_set_storage(h_wifi_storage_t storage);
h_err_t rpc_wifi_set_bandwidth(h_wifi_interface_t ifx, h_wifi_bandwidth_t bw);
h_err_t rpc_wifi_get_bandwidth(h_wifi_interface_t ifx, h_wifi_bandwidth_t *bw);
h_err_t rpc_wifi_set_channel(uint8_t primary, h_wifi_second_chan_t second);
h_err_t rpc_wifi_get_channel(uint8_t *primary, h_wifi_second_chan_t *second);
h_err_t rpc_wifi_set_country_code(const char *country, bool ieee80211d_enabled);
h_err_t rpc_wifi_get_country_code(char *country);
h_err_t rpc_wifi_set_country(const h_wifi_country_t *country);
h_err_t rpc_wifi_get_country(h_wifi_country_t *country);
h_err_t rpc_wifi_ap_get_sta_list(h_wifi_sta_list_t *sta);
h_err_t rpc_wifi_ap_get_sta_aid(const uint8_t mac[6], uint16_t *aid);
h_err_t rpc_wifi_sta_get_rssi(int *rssi);
h_err_t rpc_wifi_set_protocol(h_wifi_interface_t ifx, uint8_t protocol_bitmap);
h_err_t rpc_wifi_get_protocol(h_wifi_interface_t ifx, uint8_t *protocol_bitmap);
h_err_t rpc_wifi_set_max_tx_power(int8_t power);
h_err_t rpc_wifi_get_max_tx_power(int8_t *power);
h_err_t rpc_wifi_sta_get_negotiated_phymode(h_wifi_phy_mode_t *phymode);
h_err_t rpc_wifi_sta_get_aid(uint16_t *aid);
h_err_t rpc_wifi_set_inactive_time(h_wifi_interface_t ifx, uint16_t sec);
h_err_t rpc_wifi_get_inactive_time(h_wifi_interface_t ifx, uint16_t *sec);
h_err_t rpc_get_coprocessor_fwversion(esp_hosted_coprocessor_fwver_t *ver_info);
h_err_t rpc_get_cp_info(uint32_t *cp_chip_id, char *cp_target_name, size_t cp_target_name_len);

h_err_t rpc_bt_controller_init(void);
h_err_t rpc_bt_controller_deinit(bool mem_release);
h_err_t rpc_bt_controller_enable(void);
h_err_t rpc_bt_controller_disable(void);

h_err_t rpc_iface_mac_addr_set_get(bool set, uint8_t *mac, size_t mac_len, uint32_t type);
h_err_t rpc_iface_mac_addr_len_get(size_t *len, uint32_t type);

h_err_t rpc_iface_get_coprocessor_app_desc(esp_hosted_app_desc_t *app_desc);
h_err_t rpc_iface_configure_heartbeat(bool enable, int duration_sec);

#if H_MEM_MONITOR
h_err_t rpc_iface_set_mem_monitor(esp_hosted_config_mem_monitor_t *config, esp_hosted_curr_mem_info_t *curr_mem_info);
#endif

#if H_HOST_OT_ENABLE
h_err_t rpc_iface_openthread_rcp_init(void);
h_err_t rpc_iface_openthread_rcp_deinit(void);
h_err_t rpc_iface_openthread_rcp_start(void);
h_err_t rpc_iface_openthread_rcp_stop(void);
h_err_t rpc_iface_openthread_rcp_query(esp_hosted_openthread_query_t query);
#endif

h_err_t rpc_ota_begin(void);
h_err_t rpc_ota_write(uint8_t* ota_data, uint32_t ota_data_len);
h_err_t rpc_ota_end(void);
h_err_t rpc_ota_activate(void);

#if H_WIFI_HE_SUPPORT
h_err_t rpc_wifi_sta_twt_config(h_wifi_twt_config_t *config);
h_err_t rpc_wifi_sta_itwt_setup(h_wifi_twt_setup_config_t *setup_config);
h_err_t rpc_wifi_sta_itwt_teardown(int flow_id);
h_err_t rpc_wifi_sta_itwt_suspend(int flow_id, int suspend_time_ms);
h_err_t rpc_wifi_sta_itwt_get_flow_id_status(int *flow_id_bitmap);
h_err_t rpc_wifi_sta_itwt_send_probe_req(int timeout_ms);
h_err_t rpc_wifi_sta_itwt_set_target_wake_time_offset(int offset_us);
#endif

#if H_WIFI_DUALBAND_SUPPORT
h_err_t rpc_wifi_set_band(h_wifi_band_t band);
h_err_t rpc_wifi_get_band(h_wifi_band_t *band);
h_err_t rpc_wifi_set_band_mode(h_wifi_band_mode_t band_mode);
h_err_t rpc_wifi_get_band_mode(h_wifi_band_mode_t *band_mode);
h_err_t rpc_wifi_set_protocols(h_wifi_interface_t ifx, h_wifi_protocols_t *protocols);
h_err_t rpc_wifi_get_protocols(h_wifi_interface_t ifx, h_wifi_protocols_t *protocols);
h_err_t rpc_wifi_set_bandwidths(h_wifi_interface_t ifx, h_wifi_bandwidths_t *bw);
h_err_t rpc_wifi_get_bandwidths(h_wifi_interface_t ifx, h_wifi_bandwidths_t *bw);
#endif

h_err_t rpc_set_dhcp_dns_status(h_wifi_interface_t interface, uint8_t link_up,
		uint8_t dhcp_up, char *dhcp_ip, char *dhcp_nm, char *dhcp_gw,
		uint8_t dns_up, char *dns_ip, uint8_t dns_type);

#if H_WIFI_ENTERPRISE_SUPPORT
h_err_t rpc_wifi_sta_enterprise_enable(void);
h_err_t rpc_wifi_sta_enterprise_disable(void);
h_err_t rpc_eap_client_set_identity(const unsigned char *identity, int len);
h_err_t rpc_eap_client_clear_identity(void);
h_err_t rpc_eap_client_set_username(const unsigned char *username, int len);
h_err_t rpc_eap_client_clear_username(void);
h_err_t rpc_eap_client_set_password(const unsigned char *password, int len);
h_err_t rpc_eap_client_clear_password(void);
h_err_t rpc_eap_client_set_new_password(const unsigned char *new_password, int len);
h_err_t rpc_eap_client_clear_new_password(void);
h_err_t rpc_eap_client_set_ca_cert(const unsigned char *ca_cert, int ca_cert_len);
h_err_t rpc_eap_client_clear_ca_cert(void);

h_err_t rpc_eap_client_set_certificate_and_key(const unsigned char *client_cert, int client_cert_len,
                                                  const unsigned char *private_key, int private_key_len,
                                                  const unsigned char *private_key_password, int private_key_passwd_len);
h_err_t rpc_eap_client_clear_certificate_and_key(void);
h_err_t rpc_eap_client_set_disable_time_check(bool disable);
h_err_t rpc_eap_client_get_disable_time_check(bool *disable);
h_err_t rpc_eap_client_set_ttls_phase2_method(h_eap_ttls_phase2_types_t type);
h_err_t rpc_eap_client_set_suiteb_192bit_certification(bool enable);
h_err_t rpc_eap_client_set_pac_file(const unsigned char *pac_file, int pac_file_len);
h_err_t rpc_eap_client_set_fast_params(h_eap_fast_config_t config);
h_err_t rpc_eap_client_use_default_cert_bundle(bool use_default_bundle);
h_err_t rpc_wifi_set_okc_support(bool enable);
h_err_t rpc_eap_client_set_domain_name(const char *domain_name);
#if H_GOT_SET_EAP_METHODS_API
h_err_t rpc_eap_client_set_eap_methods(h_eap_method_t methods);
#endif
#endif
#if H_DPP_SUPPORT
#if H_SUPP_DPP_SUPPORT
h_err_t rpc_supp_dpp_init(h_supp_dpp_event_cb_t evt_cb);
#else
h_err_t rpc_supp_dpp_init(void);
#endif
h_err_t rpc_supp_dpp_deinit(void);
h_err_t rpc_supp_dpp_bootstrap_gen(const char *chan_list,
		h_supp_dpp_bootstrap_t type,
		const char *key, const char *info);
h_err_t rpc_supp_dpp_start_listen(void);
h_err_t rpc_supp_dpp_stop_listen(void);
#endif
#ifdef __cplusplus
}
#endif

#endif
