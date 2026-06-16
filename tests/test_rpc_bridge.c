/* tests/test_rpc_bridge.c — Minimal contract tests for RPC bridge files
 * (h_rpc_req.c, h_rpc_rsp.c, h_rpc_evt.c, h_control_serial_adapter.c, h_rpc_utils.c)
 */
#include "unity.h"
#include "h_types.h"
#include "h_wifi_types.h"
#include "rpc_core.h"
#include "h_control_serial_adapter.h"
#include "esp_hosted_rpc.pb-c.h"
#include <string.h>
#include <stdlib.h>

/* Forward declarations — avoid pulling old ESP-IDF headers */
extern int rpc_copy_wifi_sta_config(h_wifi_config_t *dst, WifiStaConfig *src);
extern int rpc_init(void);
extern int rpc_start(void);
extern int rpc_stop(void);
extern int rpc_deinit(void);

static void free_rpc_allocs(ctrl_cmd_t *msg)
{
    for (uint8_t i = 0; i < msg->n_rpc_free_buff_hdls; i++) {
        free(msg->rpc_free_buff_hdls[i]);
        msg->rpc_free_buff_hdls[i] = NULL;
    }
    msg->n_rpc_free_buff_hdls = 0;
}

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

void test_compose_rpc_req_wifi_set_config_sta_uses_portable_config(void)
{
    Rpc req = {0};
    ctrl_cmd_t app_req = {0};
    int32_t failure_status = 0;
    uint8_t ssid_data[] = "portable_sta";
    uint8_t pwd_data[] = "portable_pass";
    uint8_t bssid_data[] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};

    rpc__init(&req);
    req.msg_id = RPC_ID__Req_WifiSetConfig;
    app_req.u.wifi_config.iface = H_WIFI_IF_STA;
    memcpy(app_req.u.wifi_config.u.sta.ssid, ssid_data, sizeof(ssid_data) - 1);
    memcpy(app_req.u.wifi_config.u.sta.password, pwd_data, sizeof(pwd_data) - 1);
    memcpy(app_req.u.wifi_config.u.sta.bssid, bssid_data, sizeof(bssid_data));
    app_req.u.wifi_config.u.sta.ssid_len = sizeof(ssid_data) - 1;
    app_req.u.wifi_config.u.sta.channel = 11;
    app_req.u.wifi_config.u.sta.listen_interval = 7;
    app_req.u.wifi_config.u.sta.pmf_cfg_capable = 1;
    app_req.u.wifi_config.u.sta.pmf_cfg_required = 1;

    int ret = compose_rpc_req(&req, &app_req, &failure_status);

    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_EQUAL(0, failure_status);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config);
    TEST_ASSERT_EQUAL(H_WIFI_IF_STA, req.req_wifi_set_config->iface);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config->cfg);
    TEST_ASSERT_EQUAL(WIFI_CONFIG__U_STA, req.req_wifi_set_config->cfg->u_case);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config->cfg->sta);
    TEST_ASSERT_EQUAL_MEMORY(ssid_data, req.req_wifi_set_config->cfg->sta->ssid.data,
                             sizeof(ssid_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(pwd_data, req.req_wifi_set_config->cfg->sta->password.data,
                             sizeof(pwd_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(bssid_data, req.req_wifi_set_config->cfg->sta->bssid.data,
                             sizeof(bssid_data));
    TEST_ASSERT_EQUAL_UINT32(11, req.req_wifi_set_config->cfg->sta->channel);
    TEST_ASSERT_EQUAL_UINT32(7, req.req_wifi_set_config->cfg->sta->listen_interval);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config->cfg->sta->pmf_cfg);
    TEST_ASSERT_TRUE(req.req_wifi_set_config->cfg->sta->pmf_cfg->capable);
    TEST_ASSERT_TRUE(req.req_wifi_set_config->cfg->sta->pmf_cfg->required);

    free_rpc_allocs(&app_req);
}

void test_compose_rpc_req_wifi_set_config_ap_uses_portable_config(void)
{
    Rpc req = {0};
    ctrl_cmd_t app_req = {0};
    int32_t failure_status = 0;
    uint8_t ssid_data[] = "portable_ap";
    uint8_t pwd_data[] = "ap_pass";

    rpc__init(&req);
    req.msg_id = RPC_ID__Req_WifiSetConfig;
    app_req.u.wifi_config.iface = H_WIFI_IF_AP;
    memcpy(app_req.u.wifi_config.u.ap.ssid, ssid_data, sizeof(ssid_data) - 1);
    memcpy(app_req.u.wifi_config.u.ap.password, pwd_data, sizeof(pwd_data) - 1);
    app_req.u.wifi_config.u.ap.ssid_len = sizeof(ssid_data) - 1;
    app_req.u.wifi_config.u.ap.channel = 6;
    app_req.u.wifi_config.u.ap.hidden_ssid = 1;
    app_req.u.wifi_config.u.ap.max_connection = 4;
    app_req.u.wifi_config.u.ap.beacon_interval = 200;

    int ret = compose_rpc_req(&req, &app_req, &failure_status);

    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_EQUAL(0, failure_status);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config);
    TEST_ASSERT_EQUAL(H_WIFI_IF_AP, req.req_wifi_set_config->iface);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config->cfg);
    TEST_ASSERT_EQUAL(WIFI_CONFIG__U_AP, req.req_wifi_set_config->cfg->u_case);
    TEST_ASSERT_NOT_NULL(req.req_wifi_set_config->cfg->ap);
    TEST_ASSERT_EQUAL_MEMORY(ssid_data, req.req_wifi_set_config->cfg->ap->ssid.data,
                             sizeof(ssid_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(pwd_data, req.req_wifi_set_config->cfg->ap->password.data,
                             sizeof(pwd_data) - 1);
    TEST_ASSERT_EQUAL_UINT32(sizeof(ssid_data) - 1, req.req_wifi_set_config->cfg->ap->ssid_len);
    TEST_ASSERT_EQUAL_UINT32(6, req.req_wifi_set_config->cfg->ap->channel);
    TEST_ASSERT_EQUAL_UINT32(1, req.req_wifi_set_config->cfg->ap->ssid_hidden);
    TEST_ASSERT_EQUAL_UINT32(4, req.req_wifi_set_config->cfg->ap->max_connection);
    TEST_ASSERT_EQUAL_UINT32(200, req.req_wifi_set_config->cfg->ap->beacon_interval);

    free_rpc_allocs(&app_req);
}

void test_compose_rpc_req_null(void)
{
    int32_t failure_status = 0;
    /* Contract: NULL req crashes (no guard), so we only test with valid ptrs */
    (void)failure_status;
}

void test_rpc_parse_rsp_wifi_get_config_sta_populates_portable_config(void)
{
    Rpc rpc_msg = {0};
    ctrl_cmd_t app_resp = {0};
    RpcRespWifiGetConfig resp = RPC__RESP__WIFI_GET_CONFIG__INIT;
    WifiConfig cfg = WIFI_CONFIG__INIT;
    WifiStaConfig sta = WIFI_STA_CONFIG__INIT;
    WifiPmfConfig pmf = WIFI_PMF_CONFIG__INIT;
    uint8_t ssid_data[] = "rsp_sta";
    uint8_t pwd_data[] = "rsp_pass";
    uint8_t bssid_data[] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    rpc__init(&rpc_msg);
    rpc_msg.msg_id = RPC_ID__Resp_WifiGetConfig;
    rpc_msg.resp_wifi_get_config = &resp;
    resp.resp = H_OK;
    resp.iface = H_WIFI_IF_STA;
    resp.cfg = &cfg;
    cfg.u_case = WIFI_CONFIG__U_STA;
    cfg.sta = &sta;
    sta.ssid.data = ssid_data;
    sta.ssid.len = sizeof(ssid_data) - 1;
    sta.password.data = pwd_data;
    sta.password.len = sizeof(pwd_data) - 1;
    sta.bssid.data = bssid_data;
    sta.bssid.len = sizeof(bssid_data);
    sta.channel = 3;
    sta.listen_interval = 9;
    sta.pmf_cfg = &pmf;
    pmf.capable = true;
    pmf.required = true;

    int ret = rpc_parse_rsp(&rpc_msg, &app_resp);

    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_EQUAL(H_WIFI_IF_STA, app_resp.u.wifi_config.iface);
    TEST_ASSERT_EQUAL_MEMORY(ssid_data, app_resp.u.wifi_config.u.sta.ssid,
                             sizeof(ssid_data) - 1);
    TEST_ASSERT_EQUAL_UINT8(sizeof(ssid_data) - 1, app_resp.u.wifi_config.u.sta.ssid_len);
    TEST_ASSERT_EQUAL_MEMORY(pwd_data, app_resp.u.wifi_config.u.sta.password,
                             sizeof(pwd_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(bssid_data, app_resp.u.wifi_config.u.sta.bssid,
                             sizeof(bssid_data));
    TEST_ASSERT_EQUAL_UINT8(3, app_resp.u.wifi_config.u.sta.channel);
    TEST_ASSERT_EQUAL_UINT8(9, app_resp.u.wifi_config.u.sta.listen_interval);
    TEST_ASSERT_EQUAL_UINT8(1, app_resp.u.wifi_config.u.sta.pmf_cfg_capable);
    TEST_ASSERT_EQUAL_UINT8(1, app_resp.u.wifi_config.u.sta.pmf_cfg_required);
}

void test_rpc_parse_rsp_wifi_get_config_ap_populates_portable_config(void)
{
    Rpc rpc_msg = {0};
    ctrl_cmd_t app_resp = {0};
    RpcRespWifiGetConfig resp = RPC__RESP__WIFI_GET_CONFIG__INIT;
    WifiConfig cfg = WIFI_CONFIG__INIT;
    WifiApConfig ap = WIFI_AP_CONFIG__INIT;
    uint8_t ssid_data[] = "rsp_ap";
    uint8_t pwd_data[] = "rsp_ap_pass";

    rpc__init(&rpc_msg);
    rpc_msg.msg_id = RPC_ID__Resp_WifiGetConfig;
    rpc_msg.resp_wifi_get_config = &resp;
    resp.resp = H_OK;
    resp.iface = H_WIFI_IF_AP;
    resp.cfg = &cfg;
    cfg.u_case = WIFI_CONFIG__U_AP;
    cfg.ap = &ap;
    ap.ssid.data = ssid_data;
    ap.ssid.len = sizeof(ssid_data) - 1;
    ap.password.data = pwd_data;
    ap.password.len = sizeof(pwd_data) - 1;
    ap.ssid_len = sizeof(ssid_data) - 1;
    ap.channel = 8;
    ap.ssid_hidden = 1;
    ap.max_connection = 5;
    ap.beacon_interval = 300;

    int ret = rpc_parse_rsp(&rpc_msg, &app_resp);

    TEST_ASSERT_EQUAL(H_OK, ret);
    TEST_ASSERT_EQUAL(H_WIFI_IF_AP, app_resp.u.wifi_config.iface);
    TEST_ASSERT_EQUAL_MEMORY(ssid_data, app_resp.u.wifi_config.u.ap.ssid,
                             sizeof(ssid_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(pwd_data, app_resp.u.wifi_config.u.ap.password,
                             sizeof(pwd_data) - 1);
    TEST_ASSERT_EQUAL_UINT8(sizeof(ssid_data) - 1, app_resp.u.wifi_config.u.ap.ssid_len);
    TEST_ASSERT_EQUAL_UINT8(8, app_resp.u.wifi_config.u.ap.channel);
    TEST_ASSERT_EQUAL_UINT8(1, app_resp.u.wifi_config.u.ap.hidden_ssid);
    TEST_ASSERT_EQUAL_UINT8(5, app_resp.u.wifi_config.u.ap.max_connection);
    TEST_ASSERT_EQUAL_UINT16(300, app_resp.u.wifi_config.u.ap.beacon_interval);
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
    h_wifi_config_t dst = {0};
    WifiStaConfig src = WIFI_STA_CONFIG__INIT;
    WifiPmfConfig pmf = WIFI_PMF_CONFIG__INIT;

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
    src.pmf_cfg = &pmf;
    pmf.capable = true;
    pmf.required = true;
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

    /* Verify portable scalar fields were copied */
    TEST_ASSERT_EQUAL(6, dst.sta.channel);
    TEST_ASSERT_EQUAL(3, dst.sta.listen_interval);
    TEST_ASSERT_EQUAL(1, dst.sta.pmf_cfg_capable);
    TEST_ASSERT_EQUAL(1, dst.sta.pmf_cfg_required);

    /* Verify binary data was copied */
    TEST_ASSERT_EQUAL_MEMORY(ssid_data, dst.sta.ssid, sizeof(ssid_data) - 1);
    TEST_ASSERT_EQUAL_UINT8(sizeof(ssid_data) - 1, dst.sta.ssid_len);
    TEST_ASSERT_EQUAL_MEMORY(pwd_data, dst.sta.password, sizeof(pwd_data) - 1);
    TEST_ASSERT_EQUAL_MEMORY(bssid_data, dst.sta.bssid, 6);
}
