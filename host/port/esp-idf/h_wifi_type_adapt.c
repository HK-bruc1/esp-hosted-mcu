/* host/port/esp-idf/h_wifi_type_adapt.c
 *
 * Field-level bidirectional adapters + compile-time consistency checks.
 *
 * 字段映射策略见本计划 Task 3 Step 3.1 "字段保真矩阵"。
 * 若发现矩阵与代码不一致,以矩阵为准更新代码。
 */

#include "h_port_config.h"
#include "h_wifi_type_adapt.h"
#include "h_port_contract.h"
#include <string.h>
#include "esp_idf_version.h"
#include "esp_mac.h"
#include "h_dpp_types.h"
#include <stddef.h>

/* ── Compile-time coarse consistency checks ──
 * These do NOT assert field-level offsetof equality because portable
 * and native structs intentionally have different organizations.
 * Size/alignment checks are intentionally omitted: native structs vary
 * across ESP-IDF versions (e.g. v5.x vs v6.x bitfields and extra fields).
 */

/* Enum value sanity checks — cast to int to avoid -Werror=enum-compare */

/* DPP enum value checks */
#if H_DPP_SUPPORT
#include "esp_dpp.h"
_Static_assert((int)H_SUPP_DPP_EVENT_URI_READY == (int)ESP_SUPP_DPP_URI_READY,   "dpp event enum drift");
_Static_assert((int)H_SUPP_DPP_EVENT_CFG_RECVD == (int)ESP_SUPP_DPP_CFG_RECVD,   "dpp event enum drift");
_Static_assert((int)H_SUPP_DPP_EVENT_FAIL      == (int)ESP_SUPP_DPP_FAIL,        "dpp event enum drift");
_Static_assert((int)H_SUPP_DPP_BOOTSTRAP_QR_CODE == (int)ESP_SUPP_DPP_BOOTSTRAP_QR_CODE, "dpp bootstrap enum drift");
_Static_assert((int)H_SUPP_DPP_BOOTSTRAP_PKEX    == (int)ESP_SUPP_DPP_BOOTSTRAP_PKEX,    "dpp bootstrap enum drift");
#endif

/* Storage enum value checks */
_Static_assert((int)H_WIFI_STORAGE_FLASH == (int)WIFI_STORAGE_FLASH, "storage enum drift");
_Static_assert((int)H_WIFI_MODE_NULL == (int)WIFI_MODE_NULL, "mode enum drift");
_Static_assert((int)H_WIFI_MODE_STA  == (int)WIFI_MODE_STA,  "mode enum drift");
_Static_assert((int)H_WIFI_MODE_AP   == (int)WIFI_MODE_AP,   "mode enum drift");
_Static_assert((int)H_WIFI_MODE_APSTA== (int)WIFI_MODE_APSTA,"mode enum drift");

_Static_assert((int)H_WIFI_IF_STA == (int)WIFI_IF_STA, "iface enum drift");
_Static_assert((int)H_WIFI_IF_AP  == (int)WIFI_IF_AP,  "iface enum drift");

_Static_assert((int)H_WIFI_PS_NONE == (int)WIFI_PS_NONE, "ps enum drift");
_Static_assert((int)H_WIFI_PS_MIN_MODEM == (int)WIFI_PS_MIN_MODEM, "ps enum drift");

/* Bandwidth enum names changed in ESP-IDF v6.x (WIFI_BW_HT20 -> WIFI_BW20) */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
_Static_assert((int)H_WIFI_BW_HT20 == (int)WIFI_BW20, "bw enum drift");
_Static_assert((int)H_WIFI_BW_HT40 == (int)WIFI_BW40, "bw enum drift");
#else
_Static_assert((int)H_WIFI_BW_HT20 == (int)WIFI_BW_HT20, "bw enum drift");
_Static_assert((int)H_WIFI_BW_HT40 == (int)WIFI_BW_HT40, "bw enum drift");
#endif

/* ── h_wifi_init_config_t <-> wifi_init_config_t ──
 * 字段策略见矩阵:大部分双向保真,缺失字段单向补 0,feature_caps 有截断。
 */
void h_wifi_adapt_init_config_to_native(const h_wifi_init_config_t *src, wifi_init_config_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    /* 矩阵:osi_funcs / wpa_crypto_funcs — 单向(portable->native 补 NULL) */
    dst->osi_funcs = NULL;
    /* 矩阵:双向保真 */
    dst->static_rx_buf_num  = src->static_rx_buf_num;
    dst->dynamic_rx_buf_num = src->rx_buf_num;      /* 语义重命名:rx_buf_num -> dynamic_rx_buf_num */
    dst->static_tx_buf_num  = src->static_tx_buf_num;
    dst->dynamic_tx_buf_num = src->tx_buf_num;      /* 语义重命名:tx_buf_num -> dynamic_tx_buf_num */
    dst->cache_tx_buf_num   = src->cache_tx_buf_num;
    dst->csi_enable         = src->csi_enable;
    dst->ampdu_rx_enable    = src->ampdu_rx_enable;
    dst->ampdu_tx_enable    = src->ampdu_tx_enable;
    dst->nvs_enable         = src->nvs_enable;
    dst->nano_enable        = src->nano_enable;
    dst->rx_ba_win          = src->rx_ba_win_num;   /* 语义重命名 */
    dst->wifi_task_core_id  = src->wifi_task_core_id;
    dst->feature_caps       = src->feature_caps;    /* 矩阵:双向但有截断,uint32_t -> uint64_t 零扩展 */
    dst->magic              = src->magic;
    dst->mgmt_sbuf_num      = src->sta_mgmt_buf;    /* 语义重命名:sta_mgmt_buf -> mgmt_sbuf_num */
    /* tx_buf_type / rx_mgmt_buf_type / rx_mgmt_buf_num / amsdu_tx_enable /
       beacon_max_len / sta_disconnected_pm / espnow_max_encrypt_num /
       tx_hetb_queue_num / dump_hesigb_enable — 矩阵:单向,已由 memset 补 0 */
}

void h_wifi_adapt_init_config_to_host(const wifi_init_config_t *src, h_wifi_init_config_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->static_rx_buf_num  = src->static_rx_buf_num;
    dst->rx_buf_num         = src->dynamic_rx_buf_num;
    dst->static_tx_buf_num  = src->static_tx_buf_num;
    dst->tx_buf_num         = src->dynamic_tx_buf_num;
    dst->cache_tx_buf_num   = src->cache_tx_buf_num;
    dst->csi_enable         = src->csi_enable;
    dst->ampdu_rx_enable    = src->ampdu_rx_enable;
    dst->ampdu_tx_enable    = src->ampdu_tx_enable;
    dst->nvs_enable         = src->nvs_enable;
    dst->nano_enable        = src->nano_enable;
    dst->rx_ba_win_num      = src->rx_ba_win;
    dst->wifi_task_core_id  = src->wifi_task_core_id;
    /* 矩阵:feature_caps 双向但有截断,native uint64_t -> portable uint32_t */
    dst->feature_caps       = (uint32_t)src->feature_caps;
    dst->magic              = src->magic;
    dst->sta_mgmt_buf       = src->mgmt_sbuf_num;   /* 语义重命名:mgmt_sbuf_num -> sta_mgmt_buf */
    /* 缺失字段:已在 memset 中置 0,符合矩阵"单向丢弃"策略 */
}

/* ── h_wifi_config_t <-> wifi_config_t ──
 * 字段策略见矩阵:STA ssid/password/bssid/channel/listen_interval 双向保真;
 * PMF 扁平化(capable/required -> uint8_t);AP 字段双向保真(含 hidden_ssid 语义重命名);
 * 缺失字段(如 scan_method、threshold、authmode 等)由 memset 补 0。
 */
void h_wifi_adapt_config_to_native(const h_wifi_config_t *src, wifi_config_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    /* STA config */
    memcpy(dst->sta.ssid,     src->sta.ssid,     sizeof(dst->sta.ssid));
    memcpy(dst->sta.password, src->sta.password, sizeof(dst->sta.password));
    memcpy(dst->sta.bssid,    src->sta.bssid,    sizeof(dst->sta.bssid));
    dst->sta.channel = src->sta.channel;
    dst->sta.listen_interval = src->sta.listen_interval;
    /* PMF: portable flattens pmf_cfg.capable/required into uint8_t */
    dst->sta.pmf_cfg.capable  = src->sta.pmf_cfg_capable ? true : false;
    dst->sta.pmf_cfg.required = src->sta.pmf_cfg_required ? true : false;
    /* AP config */
    memcpy(dst->ap.ssid,     src->ap.ssid,     sizeof(dst->ap.ssid));
    memcpy(dst->ap.password, src->ap.password, sizeof(dst->ap.password));
    dst->ap.ssid_len        = src->ap.ssid_len;
    dst->ap.channel         = src->ap.channel;
    dst->ap.ssid_hidden     = src->ap.hidden_ssid;
    dst->ap.max_connection  = src->ap.max_connection;
    dst->ap.beacon_interval = src->ap.beacon_interval;
}

void h_wifi_adapt_config_to_host(const wifi_config_t *src, h_wifi_config_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    memcpy(dst->sta.ssid,     src->sta.ssid,     sizeof(dst->sta.ssid));
    memcpy(dst->sta.password, src->sta.password, sizeof(dst->sta.password));
    memcpy(dst->sta.bssid,    src->sta.bssid,    sizeof(dst->sta.bssid));
    dst->sta.channel           = src->sta.channel;
    dst->sta.listen_interval   = src->sta.listen_interval;
    dst->sta.pmf_cfg_capable   = src->sta.pmf_cfg.capable  ? 1 : 0;
    dst->sta.pmf_cfg_required  = src->sta.pmf_cfg.required ? 1 : 0;
    memcpy(dst->ap.ssid,     src->ap.ssid,     sizeof(dst->ap.ssid));
    memcpy(dst->ap.password, src->ap.password, sizeof(dst->ap.password));
    dst->ap.ssid_len     = src->ap.ssid_len;
    dst->ap.channel      = src->ap.channel;
    dst->ap.hidden_ssid  = src->ap.ssid_hidden;
    dst->ap.max_connection = src->ap.max_connection;
    dst->ap.beacon_interval = src->ap.beacon_interval;
}

/* ── h_wifi_scan_config_t <-> wifi_scan_config_t ──
 * 字段策略见矩阵:ssid/bssid/channel/show_hidden 双向保真;
 * 扫描时间扁平化(active.min/max、passive -> 独立字段);
 * 缺失字段(scan_type、channel_bitmap 等)由 memset 补默认值。
 */
void h_wifi_adapt_scan_config_to_native(const h_wifi_scan_config_t *src, wifi_scan_config_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->bssid     = src->bssid;
    dst->ssid      = src->ssid;
    dst->channel   = src->channel;
    dst->show_hidden = src->show_hidden;
    /* Portable flattens scan_time fields */
    dst->scan_time.active.min = src->active_scan_min_time;
    dst->scan_time.active.max = src->active_scan_max_time;
    dst->scan_time.passive    = src->passive_scan_time;
    dst->home_chan_dwell_time = src->home_chan_dwell_time;
}

void h_wifi_adapt_scan_config_to_host(const wifi_scan_config_t *src, h_wifi_scan_config_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->bssid     = src->bssid;
    dst->ssid      = src->ssid;
    dst->channel   = src->channel;
    dst->show_hidden = src->show_hidden;
    dst->active_scan_min_time = src->scan_time.active.min;
    dst->active_scan_max_time = src->scan_time.active.max;
    dst->passive_scan_time    = src->scan_time.passive;
    dst->home_chan_dwell_time = src->home_chan_dwell_time;
}

/* ── h_wifi_ap_record_t <-> wifi_ap_record_t ──
 * 字段策略见矩阵:bssid/rssi/beacon_interval/phy_.../wps 双向保真;
 * ssid 有截断(native [33] -> portable [32]);primary/second 语义重命名;
 * authmode/pairwise_cipher/group_cipher 通过 enum 适配器转换;
 * 缺失字段(ant/vht 等)由 memset 补 0(单向丢弃)。
 */
void h_wifi_adapt_ap_record_to_native(const h_wifi_ap_record_t *src, wifi_ap_record_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    memcpy(dst->bssid, src->bssid, sizeof(dst->bssid));
    memcpy(dst->ssid, src->ssid, sizeof(src->ssid));  /* native [33] vs portable [32] */
    dst->primary   = src->primary_channel;
    dst->second    = src->second_channel;
    dst->rssi      = src->rssi;
    dst->authmode  = h_wifi_adapt_auth_to_native(src->authmode);
    dst->pairwise_cipher = h_wifi_adapt_cipher_to_native(src->pairwise_cipher);
    dst->group_cipher    = h_wifi_adapt_cipher_to_native(src->group_cipher);
    memcpy(dst->country.cc, src->country, sizeof(dst->country.cc));
    dst->phy_11b = src->phy_11b;
    dst->phy_11g = src->phy_11g;
    dst->phy_11n = src->phy_11n;
    dst->phy_lr  = src->phy_lr;
    dst->wps     = src->wps;
}

void h_wifi_adapt_ap_record_to_host(const wifi_ap_record_t *src, h_wifi_ap_record_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    memcpy(dst->bssid, src->bssid, sizeof(dst->bssid));
    memcpy(dst->ssid,  src->ssid,  sizeof(dst->ssid));
    dst->ssid_len = strnlen((char*)src->ssid, sizeof(src->ssid) - 1);
    dst->primary_channel  = src->primary;
    dst->second_channel   = src->second;
    dst->rssi             = src->rssi;
    dst->authmode         = h_wifi_adapt_auth_to_host(src->authmode);
    dst->pairwise_cipher  = h_wifi_adapt_cipher_to_host(src->pairwise_cipher);
    dst->group_cipher     = h_wifi_adapt_cipher_to_host(src->group_cipher);
    memcpy(dst->country, src->country.cc, sizeof(dst->country));
    dst->country_len = 2;  /* ISO 3166-1 alpha-2 */
    dst->phy_11b = src->phy_11b;
    dst->phy_11g = src->phy_11g;
    dst->phy_11n = src->phy_11n;
    dst->phy_lr  = src->phy_lr;
    dst->wps     = src->wps;
}

/* ── h_wifi_sta_list_t <-> wifi_sta_list_t ──
 * 字段策略见矩阵:mac/rssi 双向保真;num 双向保真(需防溢出,portable 固定 10 槽位)。
 */
void h_wifi_adapt_sta_list_to_native(const h_wifi_sta_list_t *src, wifi_sta_list_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    size_t src_cap = sizeof(src->sta) / sizeof(src->sta[0]);
    size_t dst_cap = sizeof(dst->sta) / sizeof(dst->sta[0]);
    size_t n = src->num;
    if (n > src_cap) n = src_cap;
    if (n > dst_cap) n = dst_cap;
    for (size_t i = 0; i < n; i++) {
        memcpy(dst->sta[i].mac, src->sta[i].mac, sizeof(dst->sta[i].mac));
        dst->sta[i].rssi = src->sta[i].rssi;
    }
    dst->num = n;
}

void h_wifi_adapt_sta_list_to_host(const wifi_sta_list_t *src, h_wifi_sta_list_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    size_t src_cap = sizeof(src->sta) / sizeof(src->sta[0]);
    size_t dst_cap = sizeof(dst->sta) / sizeof(dst->sta[0]);
    size_t n = src->num;
    if (n > src_cap) n = src_cap;
    if (n > dst_cap) n = dst_cap;
    for (size_t i = 0; i < n; i++) {
        memcpy(dst->sta[i].mac, src->sta[i].mac, sizeof(dst->sta[i].mac));
        dst->sta[i].rssi = src->sta[i].rssi;
    }
    dst->num = n;
}

/* ── h_wifi_country_t <-> wifi_country_t ──
 * 字段策略见矩阵:cc/schan/nchan/policy 双向保真;
 * max_tx_power 有类型变化(native int8_t -> portable uint8_t)。
 */
void h_wifi_adapt_country_to_native(const h_wifi_country_t *src, wifi_country_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    memcpy(dst->cc, src->cc, sizeof(dst->cc));
    dst->schan = src->schan;
    dst->nchan = src->nchan;
    dst->max_tx_power = (int8_t)src->max_tx_power;
    dst->policy = (wifi_country_policy_t)src->policy;
}

void h_wifi_adapt_country_to_host(const wifi_country_t *src, h_wifi_country_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    memcpy(dst->cc, src->cc, sizeof(dst->cc));
    dst->schan = src->schan;
    dst->nchan = src->nchan;
    dst->max_tx_power = (uint8_t)src->max_tx_power;
    dst->policy = (uint8_t)src->policy;
}

/* ── Enum converters ── */
wifi_interface_t h_wifi_adapt_iface_to_native(h_wifi_interface_t v)
{
    switch (v) {
        case H_WIFI_IF_STA: return WIFI_IF_STA;
        case H_WIFI_IF_AP:  return WIFI_IF_AP;
        case H_WIFI_IF_NAN: return WIFI_IF_NAN;
        default:            return WIFI_IF_MAX;
    }
}

h_wifi_interface_t h_wifi_adapt_iface_to_host(wifi_interface_t v)
{
    switch (v) {
        case WIFI_IF_STA: return H_WIFI_IF_STA;
        case WIFI_IF_AP:  return H_WIFI_IF_AP;
        case WIFI_IF_NAN: return H_WIFI_IF_NAN;
        default:          return H_WIFI_IF_MAX;
    }
}

wifi_mode_t h_wifi_adapt_mode_to_native(h_wifi_mode_t v)
{
    switch (v) {
        case H_WIFI_MODE_NULL:   return WIFI_MODE_NULL;
        case H_WIFI_MODE_STA:    return WIFI_MODE_STA;
        case H_WIFI_MODE_AP:     return WIFI_MODE_AP;
        case H_WIFI_MODE_APSTA:  return WIFI_MODE_APSTA;
        case H_WIFI_MODE_NAN:    return WIFI_MODE_NAN;
        default:                 return WIFI_MODE_MAX;
    }
}

h_wifi_mode_t h_wifi_adapt_mode_to_host(wifi_mode_t v)
{
    switch (v) {
        case WIFI_MODE_NULL:   return H_WIFI_MODE_NULL;
        case WIFI_MODE_STA:    return H_WIFI_MODE_STA;
        case WIFI_MODE_AP:     return H_WIFI_MODE_AP;
        case WIFI_MODE_APSTA:  return H_WIFI_MODE_APSTA;
        case WIFI_MODE_NAN:    return H_WIFI_MODE_NAN;
        default:               return H_WIFI_MODE_NAN; /* no native MAX in portable */
    }
}

wifi_ps_type_t h_wifi_adapt_ps_to_native(h_wifi_ps_type_t v)
{
    switch (v) {
        case H_WIFI_PS_NONE:       return WIFI_PS_NONE;
        case H_WIFI_PS_MIN_MODEM:  return WIFI_PS_MIN_MODEM;
        case H_WIFI_PS_MAX_MODEM:  return WIFI_PS_MAX_MODEM;
        default:                   return WIFI_PS_NONE;
    }
}

h_wifi_ps_type_t h_wifi_adapt_ps_to_host(wifi_ps_type_t v)
{
    switch (v) {
        case WIFI_PS_NONE:      return H_WIFI_PS_NONE;
        case WIFI_PS_MIN_MODEM: return H_WIFI_PS_MIN_MODEM;
        case WIFI_PS_MAX_MODEM: return H_WIFI_PS_MAX_MODEM;
        default:                return H_WIFI_PS_NONE;
    }
}

wifi_bandwidth_t h_wifi_adapt_bw_to_native(h_wifi_bandwidth_t v)
{
    switch (v) {
        case H_WIFI_BW_HT20:
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
            return (wifi_bandwidth_t)1;  /* WIFI_BW20 */
        case H_WIFI_BW_HT40:
            return (wifi_bandwidth_t)2;  /* WIFI_BW40 */
        default:
            return (wifi_bandwidth_t)1;
#else
            return WIFI_BW_HT20;
        case H_WIFI_BW_HT40:
            return WIFI_BW_HT40;
        default:
            return WIFI_BW_HT20;
#endif
    }
}

h_wifi_bandwidth_t h_wifi_adapt_bw_to_host(wifi_bandwidth_t v)
{
    switch ((int)v) {
        case 1:  return H_WIFI_BW_HT20;  /* WIFI_BW_HT20 or WIFI_BW20 */
        case 2:  return H_WIFI_BW_HT40;  /* WIFI_BW_HT40 or WIFI_BW40 */
        default: return H_WIFI_BW_HT20;
    }
}

wifi_auth_mode_t h_wifi_adapt_auth_to_native(h_wifi_auth_mode_t v)
{
    switch (v) {
        case H_WIFI_AUTH_OPEN:          return WIFI_AUTH_OPEN;
        case H_WIFI_AUTH_WEP:           return WIFI_AUTH_WEP;
        case H_WIFI_AUTH_WPA_PSK:       return WIFI_AUTH_WPA_PSK;
        case H_WIFI_AUTH_WPA2_PSK:      return WIFI_AUTH_WPA2_PSK;
        case H_WIFI_AUTH_WPA_WPA2_PSK:  return WIFI_AUTH_WPA_WPA2_PSK;
        case H_WIFI_AUTH_WPA2_ENTERPRISE: return WIFI_AUTH_WPA2_ENTERPRISE;
        case H_WIFI_AUTH_WPA3_PSK:      return WIFI_AUTH_WPA3_PSK;
        case H_WIFI_AUTH_WPA2_WPA3_PSK: return WIFI_AUTH_WPA2_WPA3_PSK;
        case H_WIFI_AUTH_WAPI_PSK:      return WIFI_AUTH_WAPI_PSK;
        case H_WIFI_AUTH_OWE:           return WIFI_AUTH_OWE;
        case H_WIFI_AUTH_WPA3_ENT_192:  return WIFI_AUTH_WPA3_ENT_192;
        case H_WIFI_AUTH_MAX:           return WIFI_AUTH_MAX;
        default:                        return WIFI_AUTH_OPEN;
    }
}

h_wifi_auth_mode_t h_wifi_adapt_auth_to_host(wifi_auth_mode_t v)
{
    switch (v) {
        case WIFI_AUTH_OPEN:          return H_WIFI_AUTH_OPEN;
        case WIFI_AUTH_WEP:           return H_WIFI_AUTH_WEP;
        case WIFI_AUTH_WPA_PSK:       return H_WIFI_AUTH_WPA_PSK;
        case WIFI_AUTH_WPA2_PSK:      return H_WIFI_AUTH_WPA2_PSK;
        case WIFI_AUTH_WPA_WPA2_PSK:  return H_WIFI_AUTH_WPA_WPA2_PSK;
        case WIFI_AUTH_WPA2_ENTERPRISE: return H_WIFI_AUTH_WPA2_ENTERPRISE;
        case WIFI_AUTH_WPA3_PSK:      return H_WIFI_AUTH_WPA3_PSK;
        case WIFI_AUTH_WPA2_WPA3_PSK: return H_WIFI_AUTH_WPA2_WPA3_PSK;
        case WIFI_AUTH_WAPI_PSK:      return H_WIFI_AUTH_WAPI_PSK;
        case WIFI_AUTH_OWE:           return H_WIFI_AUTH_OWE;
        case WIFI_AUTH_WPA3_ENT_192:  return H_WIFI_AUTH_WPA3_ENT_192;
        case WIFI_AUTH_MAX:           return H_WIFI_AUTH_MAX;
        default:                      return H_WIFI_AUTH_OPEN;
    }
}

wifi_cipher_type_t h_wifi_adapt_cipher_to_native(h_wifi_cipher_type_t v)
{
    switch (v) {
        case H_WIFI_CIPHER_TYPE_NONE:        return WIFI_CIPHER_TYPE_NONE;
        case H_WIFI_CIPHER_TYPE_WEP40:       return WIFI_CIPHER_TYPE_WEP40;
        case H_WIFI_CIPHER_TYPE_WEP104:      return WIFI_CIPHER_TYPE_WEP104;
        case H_WIFI_CIPHER_TYPE_TKIP:        return WIFI_CIPHER_TYPE_TKIP;
        case H_WIFI_CIPHER_TYPE_CCMP:        return WIFI_CIPHER_TYPE_CCMP;
        case H_WIFI_CIPHER_TYPE_TKIP_CCMP:   return WIFI_CIPHER_TYPE_TKIP_CCMP;
        case H_WIFI_CIPHER_TYPE_AES_CMAC128: return WIFI_CIPHER_TYPE_AES_CMAC128;
        case H_WIFI_CIPHER_TYPE_SMS4:        return WIFI_CIPHER_TYPE_SMS4;
        case H_WIFI_CIPHER_TYPE_GCMP:        return WIFI_CIPHER_TYPE_GCMP;
        case H_WIFI_CIPHER_TYPE_GCMP256:     return WIFI_CIPHER_TYPE_GCMP256;
        case H_WIFI_CIPHER_TYPE_AES_GMAC128: return WIFI_CIPHER_TYPE_AES_GMAC128;
        case H_WIFI_CIPHER_TYPE_AES_GMAC256: return WIFI_CIPHER_TYPE_AES_GMAC256;
        case H_WIFI_CIPHER_TYPE_UNKNOWN:     return WIFI_CIPHER_TYPE_UNKNOWN;
        default:                             return WIFI_CIPHER_TYPE_UNKNOWN;
    }
}

h_wifi_cipher_type_t h_wifi_adapt_cipher_to_host(wifi_cipher_type_t v)
{
    switch (v) {
        case WIFI_CIPHER_TYPE_NONE:        return H_WIFI_CIPHER_TYPE_NONE;
        case WIFI_CIPHER_TYPE_WEP40:       return H_WIFI_CIPHER_TYPE_WEP40;
        case WIFI_CIPHER_TYPE_WEP104:      return H_WIFI_CIPHER_TYPE_WEP104;
        case WIFI_CIPHER_TYPE_TKIP:        return H_WIFI_CIPHER_TYPE_TKIP;
        case WIFI_CIPHER_TYPE_CCMP:        return H_WIFI_CIPHER_TYPE_CCMP;
        case WIFI_CIPHER_TYPE_TKIP_CCMP:   return H_WIFI_CIPHER_TYPE_TKIP_CCMP;
        case WIFI_CIPHER_TYPE_AES_CMAC128: return H_WIFI_CIPHER_TYPE_AES_CMAC128;
        case WIFI_CIPHER_TYPE_SMS4:        return H_WIFI_CIPHER_TYPE_SMS4;
        case WIFI_CIPHER_TYPE_GCMP:        return H_WIFI_CIPHER_TYPE_GCMP;
        case WIFI_CIPHER_TYPE_GCMP256:     return H_WIFI_CIPHER_TYPE_GCMP256;
        case WIFI_CIPHER_TYPE_AES_GMAC128: return H_WIFI_CIPHER_TYPE_AES_GMAC128;
        case WIFI_CIPHER_TYPE_AES_GMAC256: return H_WIFI_CIPHER_TYPE_AES_GMAC256;
        case WIFI_CIPHER_TYPE_UNKNOWN:     return H_WIFI_CIPHER_TYPE_UNKNOWN;
        default:                             return H_WIFI_CIPHER_TYPE_UNKNOWN;
    }
}

/* ── h_wifi_second_chan_t <-> wifi_second_chan_t ── */
_Static_assert((int)H_WIFI_SECOND_CHAN_NONE   == (int)WIFI_SECOND_CHAN_NONE,   "second_chan enum drift");
_Static_assert((int)H_WIFI_SECOND_CHAN_ABOVE  == (int)WIFI_SECOND_CHAN_ABOVE,  "second_chan enum drift");
_Static_assert((int)H_WIFI_SECOND_CHAN_BELOW  == (int)WIFI_SECOND_CHAN_BELOW,  "second_chan enum drift");

wifi_second_chan_t h_wifi_adapt_second_chan_to_native(h_wifi_second_chan_t v)
{ return (wifi_second_chan_t)v; }

h_wifi_second_chan_t h_wifi_adapt_second_chan_to_host(wifi_second_chan_t v)
{ return (h_wifi_second_chan_t)v; }

/* ── h_wifi_phy_mode_t <-> wifi_phy_mode_t ──
 * Values mirror ESP-IDF v5.3 wifi_phy_mode_t exactly — direct cast. */
_Static_assert((int)H_WIFI_PHY_MODE_LR   == (int)WIFI_PHY_MODE_LR,   "phy_mode enum drift");
_Static_assert((int)H_WIFI_PHY_MODE_11B  == (int)WIFI_PHY_MODE_11B,  "phy_mode enum drift");
_Static_assert((int)H_WIFI_PHY_MODE_11G  == (int)WIFI_PHY_MODE_11G,  "phy_mode enum drift");
_Static_assert((int)H_WIFI_PHY_MODE_11A  == (int)WIFI_PHY_MODE_11A,  "phy_mode enum drift");
_Static_assert((int)H_WIFI_PHY_MODE_HT20 == (int)WIFI_PHY_MODE_HT20, "phy_mode enum drift");
_Static_assert((int)H_WIFI_PHY_MODE_HT40 == (int)WIFI_PHY_MODE_HT40, "phy_mode enum drift");
_Static_assert((int)H_WIFI_PHY_MODE_HE20 == (int)WIFI_PHY_MODE_HE20, "phy_mode enum drift");
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
_Static_assert((int)H_WIFI_PHY_MODE_VHT20 == (int)WIFI_PHY_MODE_VHT20, "phy_mode enum drift");
#endif

wifi_phy_mode_t h_wifi_adapt_phymode_to_native(h_wifi_phy_mode_t v)
{ return (wifi_phy_mode_t)v; }

h_wifi_phy_mode_t h_wifi_adapt_phymode_to_host(wifi_phy_mode_t v)
{ return (h_wifi_phy_mode_t)v; }

/* ── h_wifi_band_t <-> wifi_band_t ──
 * Values mirror ESP-IDF wifi_band_t exactly — direct cast. */
_Static_assert((int)H_WIFI_BAND_2G   == (int)WIFI_BAND_2G,   "band enum drift");
_Static_assert((int)H_WIFI_BAND_5G   == (int)WIFI_BAND_5G,   "band enum drift");

wifi_band_t h_wifi_adapt_band_to_native(h_wifi_band_t v)
{ return (wifi_band_t)v; }

h_wifi_band_t h_wifi_adapt_band_to_host(wifi_band_t v)
{ return (h_wifi_band_t)v; }

#if H_WIFI_DUALBAND_SUPPORT
/* ── h_wifi_band_mode_t <-> wifi_band_mode_t ──
 * Values mirror ESP-IDF wifi_band_mode_t exactly — direct cast. */
_Static_assert((int)H_WIFI_BAND_MODE_2G_ONLY == (int)WIFI_BAND_MODE_2G_ONLY, "band_mode enum drift");
_Static_assert((int)H_WIFI_BAND_MODE_5G_ONLY == (int)WIFI_BAND_MODE_5G_ONLY, "band_mode enum drift");
_Static_assert((int)H_WIFI_BAND_MODE_AUTO    == (int)WIFI_BAND_MODE_AUTO,    "band_mode enum drift");

wifi_band_mode_t h_wifi_adapt_band_mode_to_native(h_wifi_band_mode_t v)
{ return (wifi_band_mode_t)v; }

h_wifi_band_mode_t h_wifi_adapt_band_mode_to_host(wifi_band_mode_t v)
{ return (h_wifi_band_mode_t)v; }

/* ── h_wifi_scan_default_params_t <-> wifi_scan_default_params_t ── */
void h_wifi_adapt_scan_default_params_to_native(const h_wifi_scan_default_params_t *src, wifi_scan_default_params_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->scan_time.active.min = src->active_scan_min_time;
    dst->scan_time.active.max = src->active_scan_max_time;
    dst->scan_time.passive    = src->passive_scan_time;
    dst->home_chan_dwell_time  = src->home_chan_dwell_time;
}

void h_wifi_adapt_scan_default_params_to_host(const wifi_scan_default_params_t *src, h_wifi_scan_default_params_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->active_scan_min_time = src->scan_time.active.min;
    dst->active_scan_max_time = src->scan_time.active.max;
    dst->passive_scan_time    = src->scan_time.passive;
    dst->home_chan_dwell_time  = src->home_chan_dwell_time;
}

/* ── h_wifi_protocols_t <-> wifi_protocols_t ── */
void h_wifi_adapt_protocols_to_native(const h_wifi_protocols_t *src, wifi_protocols_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->ghz_2g = (uint8_t)src->ghz_2g;
    dst->ghz_5g = (uint8_t)src->ghz_5g;
}

void h_wifi_adapt_protocols_to_host(const wifi_protocols_t *src, h_wifi_protocols_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->ghz_2g = src->ghz_2g;
    dst->ghz_5g = src->ghz_5g;
}

/* ── h_wifi_bandwidths_t <-> wifi_bandwidths_t ── */
void h_wifi_adapt_bandwidths_to_native(const h_wifi_bandwidths_t *src, wifi_bandwidths_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->ghz_2g = h_wifi_adapt_bw_to_native(src->ghz_2g);
    dst->ghz_5g = h_wifi_adapt_bw_to_native(src->ghz_5g);
}

void h_wifi_adapt_bandwidths_to_host(const wifi_bandwidths_t *src, h_wifi_bandwidths_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->ghz_2g = h_wifi_adapt_bw_to_host(src->ghz_2g);
    dst->ghz_5g = h_wifi_adapt_bw_to_host(src->ghz_5g);
}
#endif

/* ── h_mac_type_t <-> esp_mac_type_t ──
 * Values mirror esp_mac_type_t — direct cast. */
_Static_assert((int)H_MAC_WIFI_STA   == (int)ESP_MAC_WIFI_STA,   "mac_type enum drift");
_Static_assert((int)H_MAC_WIFI_SOFTAP == (int)ESP_MAC_WIFI_SOFTAP, "mac_type enum drift");
_Static_assert((int)H_MAC_BT         == (int)ESP_MAC_BT,         "mac_type enum drift");
_Static_assert((int)H_MAC_ETH        == (int)ESP_MAC_ETH,        "mac_type enum drift");

esp_mac_type_t h_wifi_adapt_mac_type_to_native(h_mac_type_t v)
{ return (esp_mac_type_t)v; }

h_mac_type_t h_wifi_adapt_mac_type_to_host(esp_mac_type_t v)
{ return (h_mac_type_t)v; }

/* ── h_wifi_vendor_ie_type_t <-> wifi_vendor_ie_type_t ──
 * Values mirror wifi_vendor_ie_type_t — direct cast. */
_Static_assert((int)H_WIFI_VND_IE_TYPE_BEACON     == (int)WIFI_VND_IE_TYPE_BEACON,     "vendor_ie_type enum drift");
_Static_assert((int)H_WIFI_VND_IE_TYPE_PROBE_REQ  == (int)WIFI_VND_IE_TYPE_PROBE_REQ,  "vendor_ie_type enum drift");
_Static_assert((int)H_WIFI_VND_IE_TYPE_PROBE_RESP == (int)WIFI_VND_IE_TYPE_PROBE_RESP, "vendor_ie_type enum drift");
_Static_assert((int)H_WIFI_VND_IE_TYPE_ASSOC_REQ  == (int)WIFI_VND_IE_TYPE_ASSOC_REQ,  "vendor_ie_type enum drift");
_Static_assert((int)H_WIFI_VND_IE_TYPE_ASSOC_RESP == (int)WIFI_VND_IE_TYPE_ASSOC_RESP, "vendor_ie_type enum drift");

wifi_vendor_ie_type_t h_wifi_adapt_vendor_ie_type_to_native(h_wifi_vendor_ie_type_t v)
{ return (wifi_vendor_ie_type_t)v; }

h_wifi_vendor_ie_type_t h_wifi_adapt_vendor_ie_type_to_host(wifi_vendor_ie_type_t v)
{ return (h_wifi_vendor_ie_type_t)v; }

/* ── h_wifi_vendor_ie_id_t <-> wifi_vendor_ie_id_t ──
 * Values mirror wifi_vendor_ie_id_t — direct cast. */
_Static_assert((int)H_WIFI_VND_IE_ID_0 == (int)WIFI_VND_IE_ID_0, "vendor_ie_id enum drift");
_Static_assert((int)H_WIFI_VND_IE_ID_1 == (int)WIFI_VND_IE_ID_1, "vendor_ie_id enum drift");

_Static_assert(sizeof(h_wifi_vendor_ie_data_t) == sizeof(vendor_ie_data_t), "vendor_ie_data size drift");
_Static_assert(offsetof(h_wifi_vendor_ie_data_t, element_id) == offsetof(vendor_ie_data_t, element_id),
        "vendor_ie_data element_id offset drift");
_Static_assert(offsetof(h_wifi_vendor_ie_data_t, length) == offsetof(vendor_ie_data_t, length),
        "vendor_ie_data length offset drift");
_Static_assert(offsetof(h_wifi_vendor_ie_data_t, vendor_oui) == offsetof(vendor_ie_data_t, vendor_oui),
        "vendor_ie_data vendor_oui offset drift");
_Static_assert(offsetof(h_wifi_vendor_ie_data_t, vendor_oui_type) == offsetof(vendor_ie_data_t, vendor_oui_type),
        "vendor_ie_data vendor_oui_type offset drift");
_Static_assert(offsetof(h_wifi_vendor_ie_data_t, payload) == offsetof(vendor_ie_data_t, payload),
        "vendor_ie_data payload offset drift");

wifi_vendor_ie_id_t h_wifi_adapt_vendor_ie_id_to_native(h_wifi_vendor_ie_id_t v)
{ return (wifi_vendor_ie_id_t)v; }

h_wifi_vendor_ie_id_t h_wifi_adapt_vendor_ie_id_to_host(wifi_vendor_ie_id_t v)
{ return (h_wifi_vendor_ie_id_t)v; }

/* ── Wi-Fi Conversion Contract Implementation ──
 * Core layer calls these through g_h_wifi vtable; implementations
 * delegate to the existing field-level adapters above. */

static void contract_init_config_to_req(const h_wifi_init_config_t *src, void *req_wifi_init_config)
{
    memcpy(req_wifi_init_config, src, sizeof(*src));
}

static void contract_scan_config_to_req(const h_wifi_scan_config_t *src, void *req_wifi_scan_config_cfg)
{
    memcpy(req_wifi_scan_config_cfg, src, sizeof(*src));
}

static void contract_ap_record_from_resp(const void *resp_wifi_ap_record, h_wifi_ap_record_t *dst)
{
    memcpy(dst, resp_wifi_ap_record, sizeof(*dst));
}

static void contract_ap_record_from_resp_list(const void *resp_wifi_scan_ap_list_out_list, h_wifi_ap_record_t *dst)
{
    memcpy(dst, resp_wifi_scan_ap_list_out_list, sizeof(*dst));
}

static void contract_country_to_req(const h_wifi_country_t *src, void *req_wifi_country)
{
    /* ctrl_cmd_t union stores h_wifi_country_t directly */
    memcpy(req_wifi_country, src, sizeof(h_wifi_country_t));
}

static void contract_country_from_resp(const void *resp_wifi_country, h_wifi_country_t *dst)
{
    /* ctrl_cmd_t union stores h_wifi_country_t directly */
    memcpy(dst, resp_wifi_country, sizeof(h_wifi_country_t));
}

static void contract_sta_list_from_resp(const void *resp_wifi_ap_sta_list, h_wifi_sta_list_t *dst)
{
    /* ctrl_cmd_t union stores h_wifi_sta_list_t directly */
    memcpy(dst, resp_wifi_ap_sta_list, sizeof(h_wifi_sta_list_t));
}

static uint8_t contract_iface_to_native(h_wifi_interface_t v)
{
    return (uint8_t)h_wifi_adapt_iface_to_native(v);
}

static uint8_t contract_mode_to_native(h_wifi_mode_t v)
{
    return (uint8_t)h_wifi_adapt_mode_to_native(v);
}

static uint8_t contract_ps_to_native(h_wifi_ps_type_t v)
{
    return (uint8_t)h_wifi_adapt_ps_to_native(v);
}

static uint8_t contract_bw_to_native(h_wifi_bandwidth_t v)
{
    return (uint8_t)h_wifi_adapt_bw_to_native(v);
}

static h_wifi_interface_t contract_iface_to_host(uint8_t v)
{
    return h_wifi_adapt_iface_to_host((wifi_interface_t)v);
}

static h_wifi_mode_t contract_mode_to_host(uint8_t v)
{
    return h_wifi_adapt_mode_to_host((wifi_mode_t)v);
}

static h_wifi_ps_type_t contract_ps_to_host(uint8_t v)
{
    return h_wifi_adapt_ps_to_host((wifi_ps_type_t)v);
}

static h_wifi_bandwidth_t contract_bw_to_host(uint8_t v)
{
    return h_wifi_adapt_bw_to_host((wifi_bandwidth_t)v);
}

const h_wifi_contract_t g_h_wifi = {
    .init_config_to_req       = contract_init_config_to_req,
    .scan_config_to_req       = contract_scan_config_to_req,
    .country_to_req           = contract_country_to_req,
    .ap_record_from_resp      = contract_ap_record_from_resp,
    .ap_record_from_resp_list = contract_ap_record_from_resp_list,
    .country_from_resp        = contract_country_from_resp,
    .sta_list_from_resp       = contract_sta_list_from_resp,
    .iface_to_native          = contract_iface_to_native,
    .mode_to_native           = contract_mode_to_native,
    .ps_to_native             = contract_ps_to_native,
    .bw_to_native             = contract_bw_to_native,
    .iface_to_host            = contract_iface_to_host,
    .mode_to_host             = contract_mode_to_host,
    .ps_to_host               = contract_ps_to_host,
    .bw_to_host               = contract_bw_to_host,
};

/* Compile-time storage guards for ctrl_cmd_t union members.
 * Keep each guard aligned with the actual contract memcpy target type. */
#include "rpc_slave_if.h"

_Static_assert(sizeof(h_wifi_init_config_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_init_config),
               "h_wifi_init_config_t size mismatch with ctrl_cmd_t union");
_Static_assert(sizeof(h_wifi_scan_config_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_scan_config.cfg),
               "h_wifi_scan_config_t size mismatch with ctrl_cmd_t union");
_Static_assert(sizeof(h_wifi_ap_record_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_ap_record),
               "h_wifi_ap_record_t size mismatch with ctrl_cmd_t union");

_Static_assert(sizeof(h_wifi_config_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_config.u),
               "h_wifi_config_t size mismatch with ctrl_cmd_t union");
_Static_assert(sizeof(h_wifi_country_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_country),
               "h_wifi_country_t size mismatch with ctrl_cmd_t union");
_Static_assert(sizeof(h_wifi_sta_list_t) == sizeof(((ctrl_cmd_t *)0)->u.wifi_ap_sta_list),
               "h_wifi_sta_list_t size mismatch with ctrl_cmd_t union");
