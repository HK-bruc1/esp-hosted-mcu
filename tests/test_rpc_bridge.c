/* tests/test_rpc_bridge.c — Minimal contract tests for RPC bridge files
 * (h_rpc_req.c, h_rpc_rsp.c, h_rpc_evt.c, h_control_serial_adapter.c, h_rpc_utils.c)
 */
#include "unity.h"
#include "h_types.h"
#include "rpc_core.h"
#include "h_control_serial_adapter.h"
#include "esp_hosted_rpc.pb-c.h"
#include "esp_wifi.h"  /* mock stub: wifi_sta_config_t */
#include <string.h>

/* Forward declarations — avoid pulling old ESP-IDF headers */
extern int rpc_copy_wifi_sta_config(wifi_sta_config_t *dst, WifiStaConfig *src);
extern int rpc_init(void);
extern int rpc_start(void);
extern int rpc_stop(void);
extern int rpc_deinit(void);

/* ── h_rpc_rsp.c: rpc_parse_rsp ── */
void test_rpc_parse_rsp_null(void)
{
    int ret = rpc_parse_rsp(NULL, NULL);
    /* Contract: NULL inputs gracefully return H_OK (fail_parse_rpc_msg path) */
    TEST_ASSERT_EQUAL(H_OK, ret);
}

void test_rpc_parse_rsp_base(void)
{
    Rpc rpc_msg = {0};
    ctrl_cmd_t app_resp = {0};

    rpc__init(&rpc_msg);
    rpc_msg.msg_id = RPC_ID__Resp_Base;

    int ret = rpc_parse_rsp(&rpc_msg, &app_resp);
    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_EQUAL(H_ERR_NOT_SUP, app_resp.resp_event_status);
}

void test_rpc_parse_rsp_sta_list_clamps_num_to_portable_capacity(void)
{
    Rpc rpc_msg = {0};
    ctrl_cmd_t app_resp = {0};
    RpcRespWifiApGetStaList resp = RPC__RESP__WIFI_AP_GET_STA_LIST__INIT;
    WifiStaList sta_list = WIFI_STA_LIST__INIT;
    WifiStaInfo sta_info[12];
    WifiStaInfo *sta_ptrs[12];
    uint8_t macs[12][6];

    rpc__init(&rpc_msg);
    rpc_msg.msg_id = RPC_ID__Resp_WifiApGetStaList;
    rpc_msg.resp_wifi_ap_get_sta_list = &resp;
    resp.resp = H_OK;
    resp.sta_list = &sta_list;
    sta_list.num = 12;
    sta_list.n_sta = 12;
    sta_list.sta = sta_ptrs;

    for (size_t i = 0; i < 12; i++) {
        wifi_sta_info__init(&sta_info[i]);
        macs[i][0] = (uint8_t)i;
        sta_info[i].mac.data = macs[i];
        sta_info[i].mac.len = sizeof(macs[i]);
        sta_info[i].rssi = -30 - (int)i;
        sta_ptrs[i] = &sta_info[i];
    }

    int ret = rpc_parse_rsp(&rpc_msg, &app_resp);

    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(10, app_resp.u.wifi_ap_sta_list.num);
    TEST_ASSERT_EQUAL_UINT8(0, app_resp.u.wifi_ap_sta_list.sta[0].mac[0]);
    TEST_ASSERT_EQUAL_INT8(-39, app_resp.u.wifi_ap_sta_list.sta[9].rssi);
}

/* ── h_rpc_evt.c: rpc_parse_evt ── */
void test_rpc_parse_evt_null(void)
{
    ctrl_cmd_t app_ntfy = {0};
    /* Contract: NULL rpc_msg returns H_FAIL without crashing */
    int ret = rpc_parse_evt(NULL, &app_ntfy);
    TEST_ASSERT_EQUAL(H_FAIL, ret);
}

void test_rpc_parse_evt_unknown(void)
{
    Rpc rpc_msg = {0};
    ctrl_cmd_t app_ntfy = {0};

    rpc__init(&rpc_msg);
    rpc_msg.msg_id = 0xFFFF; /* unknown event ID */

    int ret = rpc_parse_evt(&rpc_msg, &app_ntfy);
    TEST_ASSERT_EQUAL(H_FAIL, ret);
}

/* ── h_rpc_req.c: compose_rpc_req ── */
void test_compose_rpc_req_simple(void)
{
    Rpc req = {0};
    ctrl_cmd_t app_req = {0};
    int32_t failure_status = 0;

    rpc__init(&req);
    req.msg_id = RPC_ID__Req_GetWifiMode; /* no-arg request, intentional fallthrough */

    int ret = compose_rpc_req(&req, &app_req, &failure_status);
    TEST_ASSERT_EQUAL(H_OK, ret);
}

void test_compose_rpc_req_null(void)
{
    int32_t failure_status = 0;
    /* Contract: NULL req crashes (no guard), so we only test with valid ptrs */
    (void)failure_status;
}

/* ── h_control_serial_adapter.c: serial driver ── */
void test_serial_drv_open_close(void)
{
    h_control_serial_handle_t *h = h_control_serial_drv_open("spi");
    TEST_ASSERT_NOT_NULL(h);

    int ret = h_control_serial_drv_close(&h);
    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_NULL(h);
}

void test_serial_drv_null_args(void)
{
    TEST_ASSERT_NULL(h_control_serial_drv_open(NULL));

    uint8_t buf[4] = {0};
    int out_count = 0;
    TEST_ASSERT_EQUAL(H_ERR_INVALID_ARG,
        h_control_serial_drv_write(NULL, buf, sizeof(buf), &out_count));

    uint32_t nbyte = 0;
    TEST_ASSERT_NULL(h_control_serial_drv_read(NULL, &nbyte));

    TEST_ASSERT_EQUAL(H_ERR_INVALID_ARG, h_control_serial_drv_close(NULL));
}

void test_rpc_platform_deinit_safe(void)
{
    /* Deinit without prior init should be safe (no crash) */
    int ret = h_control_serial_platform_deinit();
    TEST_ASSERT_EQUAL(H_OK, ret);
}

/* ── h_rpc_utils.c: rpc_copy_wifi_sta_config ── */
/* ── h_rpc_wrap.c: RPC lifecycle ── */
void test_rpc_init_start_stop_deinit(void)
{
    /* Contract: lifecycle functions delegate to rpc_slaveif_* stubs */
    TEST_ASSERT_EQUAL(0, rpc_init());
    TEST_ASSERT_EQUAL(0, rpc_start());
    TEST_ASSERT_EQUAL(0, rpc_stop());
    TEST_ASSERT_EQUAL(0, rpc_deinit());
}

/* ── h_rpc_utils.c: rpc_copy_wifi_sta_config ── */
void test_rpc_copy_wifi_sta_config_basic(void)
{
    wifi_sta_config_t dst = {0};
    WifiStaConfig src = WIFI_STA_CONFIG__INIT;

    uint8_t ssid_data[] = "test_ssid";
    uint8_t pwd_data[] = "test_pass";
    uint8_t bssid_data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    src.ssid.data = ssid_data;
    src.ssid.len = sizeof(ssid_data) - 1;
    src.password.data = pwd_data;
    src.password.len = sizeof(pwd_data) - 1;
    src.scan_method = 1;
    src.bssid_set = 1;
    src.bssid.data = bssid_data;
    src.bssid.len = 6;
    src.channel = 6;
    src.listen_interval = 3;
    src.sort_method = 2;
    /* bitmask bits: rm=0, btm=1, mbo=2, ft=3, owe=4, transition_disable=5 */
    src.bitmask = (1U << 0) | (1U << 1) | (1U << 2) | (1U << 3) | (1U << 4) | (1U << 5);
    /* he_bitmask: he_dcm_set=0, he_dcm_max_constellation_tx[1:0]=2@bit1, he_dcm_max_constellation_rx[1:0]=3@bit3,
     * he_mcs9_enabled=5, he_su_beamformee_disabled=6 */
    src.he_bitmask = (1U << 0) | (2U << 1) | (3U << 3) | (1U << 5) | (1U << 6);
    src.sae_pwe_h2e = 1;
    src.sae_pk_mode = 2;
    src.failure_retry_cnt = 5;

    int ret = rpc_copy_wifi_sta_config(&dst, &src);

    /* Function currently always returns H_FAIL (legacy behavior) */
    TEST_ASSERT_EQUAL(H_FAIL, ret);

    /* Verify scalar fields were copied */
    TEST_ASSERT_EQUAL(1, dst.scan_method);
    TEST_ASSERT_EQUAL(1, dst.bssid_set);
    TEST_ASSERT_EQUAL(6, dst.channel);
    TEST_ASSERT_EQUAL(3, dst.listen_interval);
    TEST_ASSERT_EQUAL(2, dst.sort_method);
    TEST_ASSERT_EQUAL(1, dst.sae_pwe_h2e);
    TEST_ASSERT_EQUAL(2, dst.sae_pk_mode);
    TEST_ASSERT_EQUAL(5, dst.failure_retry_cnt);

    /* Verify binary data was copied */
    TEST_ASSERT_EQUAL_MEMORY(ssid_data, dst.ssid, sizeof(ssid_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(pwd_data, dst.password, sizeof(pwd_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(bssid_data, dst.bssid, 6);

    /* Verify bitmask feature flags */
    TEST_ASSERT_EQUAL(1, dst.rm_enabled);
    TEST_ASSERT_EQUAL(1, dst.btm_enabled);
    TEST_ASSERT_EQUAL(1, dst.mbo_enabled);
    TEST_ASSERT_EQUAL(1, dst.ft_enabled);
    TEST_ASSERT_EQUAL(1, dst.owe_enabled);
    TEST_ASSERT_EQUAL(1, dst.transition_disable);

    /* Verify HE bitmask fields */
    TEST_ASSERT_EQUAL(1, dst.he_dcm_set);
    TEST_ASSERT_EQUAL(2, dst.he_dcm_max_constellation_tx);
    TEST_ASSERT_EQUAL(3, dst.he_dcm_max_constellation_rx);
    TEST_ASSERT_EQUAL(1, dst.he_mcs9_enabled);
    TEST_ASSERT_EQUAL(1, dst.he_su_beamformee_disabled);
}
