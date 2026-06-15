/* tests/test_wifi_type_conversion.c — Field-level tests for portable Wi-Fi types
 *
 * These tests verify that the platform-independent Wi-Fi type definitions in
 * h_wifi_types.h have correct enum values, struct field layout, and
 * initialization behavior. They serve as the portable-side foundation for
 * adapter correctness: the ESP-IDF port's h_wifi_type_adapt.c uses
 * _Static_assert and switch-based mapping to guarantee enum parity at
 * compile time; these tests exercise the portable types at runtime.
 *
 * Limitation: since the Linux mock has no ESP-IDF native types, we cannot
 * directly call h_wifi_adapt_*_to_native / *_to_host here. Those are
 * verified by the ESP-IDF build's _Static_assert guards. What we CAN
 * verify is that the portable types are well-formed, have the right
 * cardinality, and survive round-trip through struct storage.
 */
#include "unity.h"
#include "h_wifi_types.h"
#include <string.h>
#include <stdint.h>

/* ── Enum cardinality and value verification ── */

void test_wifi_second_chan_enum_values(void)
{
    TEST_ASSERT_EQUAL(0, H_WIFI_SECOND_CHAN_NONE);
    TEST_ASSERT_EQUAL(1, H_WIFI_SECOND_CHAN_ABOVE);
    TEST_ASSERT_EQUAL(2, H_WIFI_SECOND_CHAN_BELOW);
}

void test_wifi_phy_mode_enum_values(void)
{
    TEST_ASSERT_EQUAL(0, H_WIFI_PHY_MODE_LR);
    TEST_ASSERT_EQUAL(1, H_WIFI_PHY_MODE_11B);
    TEST_ASSERT_EQUAL(2, H_WIFI_PHY_MODE_11G);
    TEST_ASSERT_EQUAL(3, H_WIFI_PHY_MODE_11A);
    TEST_ASSERT_EQUAL(4, H_WIFI_PHY_MODE_HT20);
    TEST_ASSERT_EQUAL(5, H_WIFI_PHY_MODE_HT40);
    TEST_ASSERT_EQUAL(6, H_WIFI_PHY_MODE_HE20);
    TEST_ASSERT_EQUAL(7, H_WIFI_PHY_MODE_VHT20);
}

void test_wifi_phy_mode_enum_distinct(void)
{
    /* All enum values must be distinct — catches copy-paste or merge errors */
    int vals[] = {
        H_WIFI_PHY_MODE_LR, H_WIFI_PHY_MODE_11B, H_WIFI_PHY_MODE_11G,
        H_WIFI_PHY_MODE_11A, H_WIFI_PHY_MODE_HT20, H_WIFI_PHY_MODE_HT40,
        H_WIFI_PHY_MODE_HE20, H_WIFI_PHY_MODE_VHT20,
    };
    int n = sizeof(vals) / sizeof(vals[0]);
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            TEST_ASSERT_NOT_EQUAL(vals[i], vals[j]);
        }
    }
}

void test_wifi_second_chan_enum_distinct(void)
{
    TEST_ASSERT_NOT_EQUAL(H_WIFI_SECOND_CHAN_NONE, H_WIFI_SECOND_CHAN_ABOVE);
    TEST_ASSERT_NOT_EQUAL(H_WIFI_SECOND_CHAN_NONE, H_WIFI_SECOND_CHAN_BELOW);
    TEST_ASSERT_NOT_EQUAL(H_WIFI_SECOND_CHAN_ABOVE, H_WIFI_SECOND_CHAN_BELOW);
}

void test_wifi_band_enum_values(void)
{
    TEST_ASSERT_EQUAL(1, H_WIFI_BAND_2G);
    TEST_ASSERT_EQUAL(2, H_WIFI_BAND_5G);
    TEST_ASSERT_EQUAL(3, H_WIFI_BAND_MAX);
}

void test_wifi_band_mode_enum_values(void)
{
    TEST_ASSERT_EQUAL(1, H_WIFI_BAND_MODE_2G_ONLY);
    TEST_ASSERT_EQUAL(2, H_WIFI_BAND_MODE_5G_ONLY);
    TEST_ASSERT_EQUAL(3, H_WIFI_BAND_MODE_AUTO);
    TEST_ASSERT_EQUAL(4, H_WIFI_BAND_MODE_MAX);
}

/* ── Scan default params struct ── */

void test_wifi_scan_default_params_sizeof(void)
{
    /* 3 x uint16_t + 1 x uint8_t = 7 bytes minimum (may have padding) */
    TEST_ASSERT_GREATER_OR_EQUAL(7, sizeof(h_wifi_scan_default_params_t));
    TEST_ASSERT_LESS_OR_EQUAL(16, sizeof(h_wifi_scan_default_params_t));
}

void test_wifi_scan_default_params_field_access(void)
{
    h_wifi_scan_default_params_t params = {
        .active_scan_min_time = 100,
        .active_scan_max_time = 300,
        .passive_scan_time = 200,
        .home_chan_dwell_time = 50,
    };
    TEST_ASSERT_EQUAL_UINT16(100, params.active_scan_min_time);
    TEST_ASSERT_EQUAL_UINT16(300, params.active_scan_max_time);
    TEST_ASSERT_EQUAL_UINT16(200, params.passive_scan_time);
    TEST_ASSERT_EQUAL_UINT8(50, params.home_chan_dwell_time);
}

void test_wifi_scan_default_params_boundary(void)
{
    /* Max values should survive storage without truncation */
    h_wifi_scan_default_params_t params = {
        .active_scan_min_time = UINT16_MAX,
        .active_scan_max_time = UINT16_MAX,
        .passive_scan_time = UINT16_MAX,
        .home_chan_dwell_time = 255,
    };
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, params.active_scan_min_time);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, params.active_scan_max_time);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, params.passive_scan_time);
    TEST_ASSERT_EQUAL_UINT8(255, params.home_chan_dwell_time);
}

void test_wifi_scan_default_params_zero_init(void)
{
    h_wifi_scan_default_params_t params;
    memset(&params, 0, sizeof(params));
    TEST_ASSERT_EQUAL_UINT16(0, params.active_scan_min_time);
    TEST_ASSERT_EQUAL_UINT16(0, params.active_scan_max_time);
    TEST_ASSERT_EQUAL_UINT16(0, params.passive_scan_time);
    TEST_ASSERT_EQUAL_UINT8(0, params.home_chan_dwell_time);
}

/* ── Enum round-trip through struct fields ──
 * Simulates the storage pattern used in rpc_wifi_channel_t and
 * rpc_wifi_sta_get_negotiated_phymode_t: enum stored in a struct,
 * read back, mutated, read again. */

void test_wifi_second_chan_round_trip(void)
{
    typedef struct {
        uint8_t primary;
        h_wifi_second_chan_t second;
    } test_channel_t;

    test_channel_t ch = { .primary = 6, .second = H_WIFI_SECOND_CHAN_ABOVE };
    TEST_ASSERT_EQUAL(H_WIFI_SECOND_CHAN_ABOVE, ch.second);
    TEST_ASSERT_EQUAL_UINT8(6, ch.primary);

    ch.second = H_WIFI_SECOND_CHAN_BELOW;
    TEST_ASSERT_EQUAL(H_WIFI_SECOND_CHAN_BELOW, ch.second);

    ch.second = H_WIFI_SECOND_CHAN_NONE;
    TEST_ASSERT_EQUAL(H_WIFI_SECOND_CHAN_NONE, ch.second);
}

void test_wifi_phy_mode_round_trip(void)
{
    typedef struct {
        h_wifi_phy_mode_t phymode;
    } test_phymode_t;

    /* Store each value, read back, verify no corruption */
    h_wifi_phy_mode_t all_modes[] = {
        H_WIFI_PHY_MODE_LR, H_WIFI_PHY_MODE_11B, H_WIFI_PHY_MODE_11G,
        H_WIFI_PHY_MODE_11A, H_WIFI_PHY_MODE_HT20, H_WIFI_PHY_MODE_HT40,
        H_WIFI_PHY_MODE_HE20, H_WIFI_PHY_MODE_VHT20,
    };
    for (size_t i = 0; i < sizeof(all_modes) / sizeof(all_modes[0]); i++) {
        test_phymode_t pm = { .phymode = all_modes[i] };
        TEST_ASSERT_EQUAL(all_modes[i], pm.phymode);
    }
}

/* ── h_wifi_config_t field access ── */

void test_wifi_config_t_sta_fields(void)
{
    h_wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    memcpy(cfg.sta.ssid, "TestSSID", 8);
    cfg.sta.ssid_len = 8;
    memcpy(cfg.sta.password, "password123", 11);
    cfg.sta.channel = 6;
    cfg.sta.listen_interval = 3;
    cfg.sta.pmf_cfg_capable = 1;
    cfg.sta.pmf_cfg_required = 0;

    TEST_ASSERT_EQUAL_UINT8(8, cfg.sta.ssid_len);
    TEST_ASSERT_EQUAL_UINT8(6, cfg.sta.channel);
    TEST_ASSERT_EQUAL_STRING("TestSSID", (char *)cfg.sta.ssid);
    TEST_ASSERT_EQUAL_UINT8(3, cfg.sta.listen_interval);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.sta.pmf_cfg_capable);
    TEST_ASSERT_EQUAL_UINT8(0, cfg.sta.pmf_cfg_required);
}

void test_wifi_config_t_ap_fields(void)
{
    h_wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    memcpy(cfg.ap.ssid, "MyAP", 4);
    cfg.ap.ssid_len = 4;
    cfg.ap.channel = 11;
    cfg.ap.hidden_ssid = 1;
    cfg.ap.max_connection = 4;
    cfg.ap.beacon_interval = 100;

    TEST_ASSERT_EQUAL_UINT8(4, cfg.ap.ssid_len);
    TEST_ASSERT_EQUAL_UINT8(11, cfg.ap.channel);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.ap.hidden_ssid);
    TEST_ASSERT_EQUAL_UINT8(4, cfg.ap.max_connection);
    TEST_ASSERT_EQUAL_UINT16(100, cfg.ap.beacon_interval);
}

void test_wifi_config_t_sta_ap_isolation(void)
{
    /* Writing to STA fields must not corrupt AP fields and vice versa */
    h_wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    memcpy(cfg.sta.ssid, "STA_SSID", 8);
    cfg.sta.channel = 1;

    memcpy(cfg.ap.ssid, "AP_SSID", 7);
    cfg.ap.channel = 11;

    TEST_ASSERT_EQUAL_STRING("STA_SSID", (char *)cfg.sta.ssid);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.sta.channel);
    TEST_ASSERT_EQUAL_STRING("AP_SSID", (char *)cfg.ap.ssid);
    TEST_ASSERT_EQUAL_UINT8(11, cfg.ap.channel);
}

/* ── h_wifi_protocols_t ── */

void test_wifi_protocols_t_field_access(void)
{
    h_wifi_protocols_t prot = {
        .ghz_2g = 0x0001,
        .ghz_5g = 0x0002,
    };
    TEST_ASSERT_EQUAL_UINT16(0x0001, prot.ghz_2g);
    TEST_ASSERT_EQUAL_UINT16(0x0002, prot.ghz_5g);
}

void test_wifi_protocols_t_boundary(void)
{
    h_wifi_protocols_t prot = {
        .ghz_2g = UINT16_MAX,
        .ghz_5g = UINT16_MAX,
    };
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, prot.ghz_2g);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, prot.ghz_5g);
}

void test_wifi_protocols_t_zero_init(void)
{
    h_wifi_protocols_t prot;
    memset(&prot, 0, sizeof(prot));
    TEST_ASSERT_EQUAL_UINT16(0, prot.ghz_2g);
    TEST_ASSERT_EQUAL_UINT16(0, prot.ghz_5g);
}

/* ── h_wifi_bandwidths_t ── */

void test_wifi_bandwidths_t_field_access(void)
{
    h_wifi_bandwidths_t bw = {
        .ghz_2g = H_WIFI_BW_HT20,
        .ghz_5g = H_WIFI_BW_HT40,
    };
    TEST_ASSERT_EQUAL(H_WIFI_BW_HT20, bw.ghz_2g);
    TEST_ASSERT_EQUAL(H_WIFI_BW_HT40, bw.ghz_5g);
}

void test_wifi_bandwidths_t_zero_init(void)
{
    h_wifi_bandwidths_t bw;
    memset(&bw, 0, sizeof(bw));
    TEST_ASSERT_EQUAL(0, bw.ghz_2g);
    TEST_ASSERT_EQUAL(0, bw.ghz_5g);
}

void test_wifi_sta_list_capacity_is_ten(void)
{
    h_wifi_sta_list_t list;
    TEST_ASSERT_EQUAL(10, sizeof(list.sta) / sizeof(list.sta[0]));
}

/* ── h_wifi_twt_config_t ── */

void test_wifi_twt_config_t_field_access(void)
{
    h_wifi_twt_config_t cfg = {
        .post_wakeup_event = true,
        .twt_enable_keep_alive = false,
    };
    TEST_ASSERT_TRUE(cfg.post_wakeup_event);
    TEST_ASSERT_FALSE(cfg.twt_enable_keep_alive);
}

void test_wifi_twt_config_t_zero_init(void)
{
    h_wifi_twt_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    TEST_ASSERT_FALSE(cfg.post_wakeup_event);
    TEST_ASSERT_FALSE(cfg.twt_enable_keep_alive);
}

/* ── h_mac_type_t ── */

void test_mac_type_enum_values(void)
{
    TEST_ASSERT_EQUAL(0, H_MAC_WIFI_STA);
    TEST_ASSERT_EQUAL(1, H_MAC_WIFI_SOFTAP);
    TEST_ASSERT_EQUAL(2, H_MAC_BT);
    TEST_ASSERT_EQUAL(3, H_MAC_ETH);
}

void test_mac_type_enum_distinct(void)
{
    TEST_ASSERT_NOT_EQUAL(H_MAC_WIFI_STA, H_MAC_WIFI_SOFTAP);
    TEST_ASSERT_NOT_EQUAL(H_MAC_WIFI_STA, H_MAC_BT);
    TEST_ASSERT_NOT_EQUAL(H_MAC_WIFI_STA, H_MAC_ETH);
    TEST_ASSERT_NOT_EQUAL(H_MAC_WIFI_SOFTAP, H_MAC_BT);
    TEST_ASSERT_NOT_EQUAL(H_MAC_WIFI_SOFTAP, H_MAC_ETH);
    TEST_ASSERT_NOT_EQUAL(H_MAC_BT, H_MAC_ETH);
}

/* ── h_wifi_vendor_ie_type_t ── */

void test_wifi_vendor_ie_type_enum_values(void)
{
    TEST_ASSERT_EQUAL(0, H_WIFI_VND_IE_TYPE_BEACON);
    TEST_ASSERT_EQUAL(1, H_WIFI_VND_IE_TYPE_PROBE_REQ);
    TEST_ASSERT_EQUAL(2, H_WIFI_VND_IE_TYPE_PROBE_RESP);
    TEST_ASSERT_EQUAL(3, H_WIFI_VND_IE_TYPE_ASSOC_REQ);
    TEST_ASSERT_EQUAL(4, H_WIFI_VND_IE_TYPE_ASSOC_RESP);
}

/* ── h_wifi_vendor_ie_id_t ── */

void test_wifi_vendor_ie_id_enum_values(void)
{
    TEST_ASSERT_EQUAL(0, H_WIFI_VND_IE_ID_0);
    TEST_ASSERT_EQUAL(1, H_WIFI_VND_IE_ID_1);
}
