/*
 * ESP-Hosted Host Core — Platform-Independent Wi-Fi Types
 *
 * These structs provide a stable, platform-independent API surface
 * for the host core layer. They are NOT binary-compatible with
 * ESP-IDF native wifi_* types; port layers must use explicit field-
 * level adapters (see host/port/esp-idf/h_wifi_type_adapt.c).
 */

#ifndef H_WIFI_TYPES_H
#define H_WIFI_TYPES_H

#include "h_types.h"
#include <stdint.h>
#include <stdbool.h>

/* ── Wi-Fi Initialization Config ── */
typedef struct {
    uint32_t  feature_caps;    /* Feature capability flags */
    uint32_t  sta_mgmt_buf;    /* mgmt buf for STA */
    uint16_t  tx_buf_num;      /* dynamic TX buffer number */
    uint16_t  rx_buf_num;      /* dynamic RX buffer number */
    uint8_t   static_rx_buf_num;
    uint8_t   static_tx_buf_num;
    uint8_t   cache_tx_buf_num;
    uint8_t   csi_enable;
    uint8_t   ampdu_rx_enable;
    uint8_t   ampdu_tx_enable;
    uint8_t   nvs_enable;
    uint8_t   nano_enable;
    uint32_t  magic;           /* WIFI_INIT_CONFIG_MAGIC equivalent */
    uint8_t   rx_ba_win_num;
    uint8_t   tx_ba_win_num;
    uint8_t   rx_ba_win_num_11ax;
    uint8_t   tx_ba_win_num_11ax;
    int       wifi_task_core_id;
} h_wifi_init_config_t;

/* ── Wi-Fi Configuration ──
 * Replaces wifi_config_t (union of sta_config and ap_config).
 * Phase 1: fields cover STA and SoftAP modes. */
typedef struct {
    /* STA config */
    struct {
        uint8_t  ssid[32];
        uint8_t  ssid_len;
        uint8_t  password[64];
        uint8_t  bssid[6];
        uint8_t  channel;
        uint8_t  listen_interval;
        uint8_t  pmf_cfg_capable;
        uint8_t  pmf_cfg_required;
    } sta;

    /* AP config */
    struct {
        uint8_t  ssid[32];
        uint8_t  ssid_len;
        uint8_t  password[64];
        uint8_t  channel;
        uint8_t  hidden_ssid;
        uint8_t  max_connection;
        uint16_t beacon_interval;
    } ap;
} h_wifi_config_t;

/* ── Scan Config ──
 * Replaces wifi_scan_config_t. */
typedef struct {
    uint8_t  *bssid;          /* NULL = scan all */
    uint8_t  *ssid;           /* NULL = scan all */
    uint8_t  ssid_len;
    uint8_t  channel;         /* 0 = scan all channels */
    bool     show_hidden;
    uint16_t active_scan_min_time;
    uint16_t active_scan_max_time;
    uint16_t passive_scan_time;
    uint8_t  home_chan_dwell_time;
} h_wifi_scan_config_t;

/* ── AP Record (Scan Result) ──
 * Replaces wifi_ap_record_t. Phase 1: essential fields only. */
typedef struct {
    uint8_t  bssid[6];
    uint8_t  ssid[32];
    uint8_t  ssid_len;
    uint8_t  primary_channel;
    uint8_t  second_channel;
    int8_t   rssi;
    uint8_t  authmode;
    uint8_t  pairwise_cipher;
    uint8_t  group_cipher;
    uint16_t beacon_interval;
    uint8_t  country[3];
    uint8_t  country_len;
    uint8_t  phy_11b : 1;
    uint8_t  phy_11g : 1;
    uint8_t  phy_11n : 1;
    uint8_t  phy_lr  : 1;
    uint8_t  wps     : 1;
    uint8_t  reserved : 3;
} h_wifi_ap_record_t;

/* ── STA List ──
 * Replaces wifi_sta_list_t. */
typedef struct {
    struct {
        uint8_t  mac[6];
        int8_t   rssi;
    } sta[10];              /* Phase 1: max 10 stations */
    uint8_t num;
} h_wifi_sta_list_t;

/* ── Country Config ──
 * Replaces wifi_country_t. */
typedef struct {
    uint8_t cc[3];           /* ISO 3166-1 alpha-2 + '\0' */
    uint8_t schan;           /* start channel */
    uint8_t nchan;           /* total channels */
    uint8_t max_tx_power;    /* dBm */
    uint8_t policy;          /* country policy */
} h_wifi_country_t;

/* ── Interface / Mode / Policy Enums ── */
typedef enum {
    H_WIFI_IF_STA = 0,
    H_WIFI_IF_AP,
    H_WIFI_IF_NAN,
    H_WIFI_IF_MAX,
} h_wifi_interface_t;

typedef enum {
    H_WIFI_MODE_NULL = 0,
    H_WIFI_MODE_STA,
    H_WIFI_MODE_AP,
    H_WIFI_MODE_APSTA,
    H_WIFI_MODE_NAN,
} h_wifi_mode_t;

typedef enum {
    H_WIFI_PS_NONE = 0,
    H_WIFI_PS_MIN_MODEM,
    H_WIFI_PS_MAX_MODEM,
} h_wifi_ps_type_t;

typedef enum {
    H_WIFI_BW_HT20 = 1,
    H_WIFI_BW_HT40 = 2,
} h_wifi_bandwidth_t;

/* ── Second Channel ──
 * Replaces wifi_second_chan_t. */
typedef enum {
    H_WIFI_SECOND_CHAN_NONE = 0,
    H_WIFI_SECOND_CHAN_ABOVE = 1,
    H_WIFI_SECOND_CHAN_BELOW = 2,
} h_wifi_second_chan_t;

/* ── PHY Mode ──
 * Replaces the co-processor platform's PHY mode enum.
 * Values are chosen for direct cast compatibility; the port adapter
 * uses _Static_assert to catch enum drift at compile time. */
typedef enum {
    H_WIFI_PHY_MODE_LR   = 0,
    H_WIFI_PHY_MODE_11B  = 1,
    H_WIFI_PHY_MODE_11G  = 2,
    H_WIFI_PHY_MODE_11A  = 3,
    H_WIFI_PHY_MODE_HT20 = 4,
    H_WIFI_PHY_MODE_HT40 = 5,
    H_WIFI_PHY_MODE_HE20 = 6,
    H_WIFI_PHY_MODE_VHT20 = 7,
} h_wifi_phy_mode_t;

/* ── Band ──
 * Replaces wifi_band_t. */
typedef enum {
    H_WIFI_BAND_2G   = 1,
    H_WIFI_BAND_5G   = 2,
    H_WIFI_BAND_MAX  = 3, /* Host-local sentinel; no ESP-IDF native equivalent. */
} h_wifi_band_t;

/* ── Band Mode ──
 * Replaces wifi_band_mode_t. */
typedef enum {
    H_WIFI_BAND_MODE_2G_ONLY = 1,
    H_WIFI_BAND_MODE_5G_ONLY = 2,
    H_WIFI_BAND_MODE_AUTO = 3,
    H_WIFI_BAND_MODE_MAX = 4, /* Host-local sentinel; no ESP-IDF native equivalent. */
} h_wifi_band_mode_t;

/* ── Auth Mode ──
 * Replaces wifi_auth_mode_t. */
typedef enum {
    H_WIFI_AUTH_OPEN = 0,
    H_WIFI_AUTH_WEP,
    H_WIFI_AUTH_WPA_PSK,
    H_WIFI_AUTH_WPA2_PSK,
    H_WIFI_AUTH_WPA_WPA2_PSK,
    H_WIFI_AUTH_WPA2_ENTERPRISE,
    H_WIFI_AUTH_WPA3_PSK,
    H_WIFI_AUTH_WPA2_WPA3_PSK,
    H_WIFI_AUTH_WAPI_PSK,
    H_WIFI_AUTH_OWE,
    H_WIFI_AUTH_WPA3_ENT_192,
    H_WIFI_AUTH_MAX
} h_wifi_auth_mode_t;

/* ── Cipher Type ──
 * Replaces wifi_cipher_type_t. */
typedef enum {
    H_WIFI_CIPHER_TYPE_NONE = 0,
    H_WIFI_CIPHER_TYPE_WEP40,
    H_WIFI_CIPHER_TYPE_WEP104,
    H_WIFI_CIPHER_TYPE_TKIP,
    H_WIFI_CIPHER_TYPE_CCMP,
    H_WIFI_CIPHER_TYPE_TKIP_CCMP,
    H_WIFI_CIPHER_TYPE_AES_CMAC128,
    H_WIFI_CIPHER_TYPE_SMS4,
    H_WIFI_CIPHER_TYPE_GCMP,
    H_WIFI_CIPHER_TYPE_GCMP256,
    H_WIFI_CIPHER_TYPE_AES_GMAC128,
    H_WIFI_CIPHER_TYPE_AES_GMAC256,
    H_WIFI_CIPHER_TYPE_UNKNOWN,
} h_wifi_cipher_type_t;

/* ── Storage Type ──
 * Replaces wifi_storage_t. */
typedef enum {
	H_WIFI_STORAGE_FLASH = 0,
	H_WIFI_STORAGE_RAM   = 1,
} h_wifi_storage_t;

/* ── Vendor IE Type ──
 * Replaces wifi_vendor_ie_type_t (esp_wifi_types.h). */
typedef enum {
    H_WIFI_VND_IE_TYPE_BEACON = 0,
    H_WIFI_VND_IE_TYPE_PROBE_REQ,
    H_WIFI_VND_IE_TYPE_PROBE_RESP,
    H_WIFI_VND_IE_TYPE_ASSOC_REQ,
    H_WIFI_VND_IE_TYPE_ASSOC_RESP,
} h_wifi_vendor_ie_type_t;

/* ── Vendor IE ID ──
 * Replaces wifi_vendor_ie_id_t (esp_wifi_types.h). */
typedef enum {
    H_WIFI_VND_IE_ID_0 = 0,
    H_WIFI_VND_IE_ID_1,
} h_wifi_vendor_ie_id_t;

/* Vendor IE Data
 * Replaces vendor_ie_data_t (esp_wifi_types.h). */
typedef struct {
    uint8_t element_id;
    uint8_t length;
    uint8_t vendor_oui[3];
    uint8_t vendor_oui_type;
    uint8_t payload[0];
} h_wifi_vendor_ie_data_t;

/* ── TWT Setup Config ──
 * Replaces wifi_itwt_setup_config_t (IDF > 5.3) and
 * wifi_twt_setup_config_t (IDF <= 5.3). Unified portable version. */
typedef struct {
	uint8_t  setup_cmd;
	uint8_t  trigger;
	uint8_t  flow_type;
	uint8_t  flow_id;
	uint8_t  wake_invl_expn;
	uint8_t  wake_duration_unit;
	uint8_t  reserved;
	uint8_t  min_wake_dura;
	uint16_t wake_invl_mant;
	uint8_t  twt_id;
	uint32_t timeout_time_ms;
} h_wifi_twt_setup_config_t;

/* ── TWT Config ──
 * Replaces wifi_twt_config_t (esp_wifi_types.h).
 * Only the fields used by RPC serialization are included. */
typedef struct {
    bool post_wakeup_event;
    bool twt_enable_keep_alive;
} h_wifi_twt_config_t;

/* ── Scan Default Params ──
 * Replaces wifi_scan_default_params_t. */
typedef struct {
    uint16_t active_scan_min_time;   /* minimum active scan time per channel (ms) */
    uint16_t active_scan_max_time;   /* maximum active scan time per channel (ms) */
    uint16_t passive_scan_time;      /* passive scan time per channel (ms) */
    uint8_t  home_chan_dwell_time;   /* dwell time on home channel (ms) */
} h_wifi_scan_default_params_t;

/* ── Protocols Config ──
 * Replaces wifi_protocols_t. Protocol bitmaps per band. */
typedef struct {
    uint16_t ghz_2g;             /* 2.4 GHz protocol bitmap */
    uint16_t ghz_5g;             /* 5 GHz protocol bitmap */
} h_wifi_protocols_t;

/* ── Bandwidths Config ──
 * Replaces wifi_bandwidths_t. Bandwidth setting per band. */
typedef struct {
    h_wifi_bandwidth_t ghz_2g;   /* 2.4 GHz bandwidth */
    h_wifi_bandwidth_t ghz_5g;   /* 5 GHz bandwidth */
} h_wifi_bandwidths_t;

#endif /* H_WIFI_TYPES_H */
