/* tests/test_runner.c — Test runner entry point */
#include "unity.h"

/* Declare test suites */
extern void test_osal_malloc_free(void);
extern void test_osal_mutex_lock_unlock(void);
extern void test_osal_semaphore(void);
extern void test_osal_thread(void);
extern void test_osal_queue(void);
extern void test_osal_init_deinit_pair(void);
extern void test_vtable_null_protection(void);
extern void test_err_code_translation(void);
extern void test_rpc_request_timeout(void);
extern void test_rpc_request_response_match(void);
extern void test_event_register_and_post(void);
extern void test_event_multiple_handlers(void);
extern void test_transport_init_spi(void);
extern void test_transport_mock_transfer(void);
extern void test_transport_drv_state_ready(void);
extern void test_teardown_transport_safe(void);
extern void test_transport_drv_remove_channel_null(void);
extern void test_process_priv_communication_null(void);
/* RPC bridge tests */
extern void test_rpc_parse_rsp_null(void);
extern void test_rpc_parse_rsp_base(void);
extern void test_rpc_parse_rsp_sta_list_clamps_num_to_portable_capacity(void);
extern void test_rpc_parse_evt_null(void);
extern void test_rpc_parse_evt_unknown(void);
extern void test_compose_rpc_req_simple(void);
extern void test_serial_drv_open_close(void);
extern void test_serial_drv_null_args(void);
extern void test_rpc_platform_deinit_safe(void);
extern void test_rpc_init_start_stop_deinit(void);
extern void test_rpc_copy_wifi_sta_config_basic(void);
/* Wi-Fi type conversion tests */
extern void test_wifi_second_chan_enum_values(void);
extern void test_wifi_phy_mode_enum_values(void);
extern void test_wifi_phy_mode_enum_distinct(void);
extern void test_wifi_second_chan_enum_distinct(void);
extern void test_wifi_band_enum_values(void);
extern void test_wifi_band_mode_enum_values(void);
extern void test_wifi_scan_default_params_sizeof(void);
extern void test_wifi_scan_default_params_field_access(void);
extern void test_wifi_scan_default_params_boundary(void);
extern void test_wifi_scan_default_params_zero_init(void);
extern void test_wifi_second_chan_round_trip(void);
extern void test_wifi_phy_mode_round_trip(void);
extern void test_wifi_config_t_sta_fields(void);
extern void test_wifi_config_t_ap_fields(void);
extern void test_wifi_config_t_sta_ap_isolation(void);
extern void test_wifi_protocols_t_field_access(void);
extern void test_wifi_protocols_t_boundary(void);
extern void test_wifi_protocols_t_zero_init(void);
extern void test_wifi_bandwidths_t_field_access(void);
extern void test_wifi_bandwidths_t_zero_init(void);
extern void test_wifi_sta_list_capacity_is_ten(void);
extern void test_wifi_twt_config_t_field_access(void);
extern void test_wifi_twt_config_t_zero_init(void);
extern void test_mac_type_enum_values(void);
extern void test_mac_type_enum_distinct(void);
extern void test_wifi_vendor_ie_type_enum_values(void);
extern void test_wifi_vendor_ie_id_enum_values(void);

void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    /* OSAL tests */
    RUN_TEST(test_osal_malloc_free);
    RUN_TEST(test_osal_mutex_lock_unlock);
    RUN_TEST(test_osal_semaphore);
    RUN_TEST(test_osal_thread);
    RUN_TEST(test_osal_queue);
    RUN_TEST(test_osal_init_deinit_pair);
    RUN_TEST(test_vtable_null_protection);
    RUN_TEST(test_err_code_translation);
    /* RPC core tests */
    RUN_TEST(test_rpc_request_timeout);
    RUN_TEST(test_rpc_request_response_match);
    /* Event tests */
    RUN_TEST(test_event_register_and_post);
    RUN_TEST(test_event_multiple_handlers);
    /* Transport tests */
    RUN_TEST(test_transport_init_spi);
    RUN_TEST(test_transport_mock_transfer);
    RUN_TEST(test_transport_drv_state_ready);
    RUN_TEST(test_teardown_transport_safe);
    RUN_TEST(test_transport_drv_remove_channel_null);
    RUN_TEST(test_process_priv_communication_null);
    /* RPC bridge contract tests */
    RUN_TEST(test_rpc_parse_rsp_null);
    RUN_TEST(test_rpc_parse_rsp_base);
    RUN_TEST(test_rpc_parse_rsp_sta_list_clamps_num_to_portable_capacity);
    RUN_TEST(test_rpc_parse_evt_null);
    RUN_TEST(test_rpc_parse_evt_unknown);
    RUN_TEST(test_compose_rpc_req_simple);
    RUN_TEST(test_serial_drv_open_close);
    RUN_TEST(test_serial_drv_null_args);
    RUN_TEST(test_rpc_platform_deinit_safe);
    RUN_TEST(test_rpc_init_start_stop_deinit);
    RUN_TEST(test_rpc_copy_wifi_sta_config_basic);
    /* Wi-Fi type conversion tests */
    RUN_TEST(test_wifi_second_chan_enum_values);
    RUN_TEST(test_wifi_phy_mode_enum_values);
    RUN_TEST(test_wifi_phy_mode_enum_distinct);
    RUN_TEST(test_wifi_second_chan_enum_distinct);
    RUN_TEST(test_wifi_band_enum_values);
    RUN_TEST(test_wifi_band_mode_enum_values);
    RUN_TEST(test_wifi_scan_default_params_sizeof);
    RUN_TEST(test_wifi_scan_default_params_field_access);
    RUN_TEST(test_wifi_scan_default_params_boundary);
    RUN_TEST(test_wifi_scan_default_params_zero_init);
    RUN_TEST(test_wifi_second_chan_round_trip);
    RUN_TEST(test_wifi_phy_mode_round_trip);
    RUN_TEST(test_wifi_config_t_sta_fields);
    RUN_TEST(test_wifi_config_t_ap_fields);
    RUN_TEST(test_wifi_config_t_sta_ap_isolation);
    RUN_TEST(test_wifi_protocols_t_field_access);
    RUN_TEST(test_wifi_protocols_t_boundary);
    RUN_TEST(test_wifi_protocols_t_zero_init);
    RUN_TEST(test_wifi_bandwidths_t_field_access);
    RUN_TEST(test_wifi_bandwidths_t_zero_init);
    RUN_TEST(test_wifi_sta_list_capacity_is_ten);
    RUN_TEST(test_wifi_twt_config_t_field_access);
    RUN_TEST(test_wifi_twt_config_t_zero_init);
    RUN_TEST(test_mac_type_enum_values);
    RUN_TEST(test_mac_type_enum_distinct);
    RUN_TEST(test_wifi_vendor_ie_type_enum_values);
    RUN_TEST(test_wifi_vendor_ie_id_enum_values);
    return UNITY_END();
}
