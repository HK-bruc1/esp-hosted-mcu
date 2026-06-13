/* host/core/include/h_public/h_event.h */
#ifndef H_EVENT_H
#define H_EVENT_H

#include "h_types.h"
#include "h_wifi_types.h"
#include "h_config.h"

/* ── Wi-Fi Event IDs ──
 * Sub-IDs for H_EVENT_WIFI base. Replaces WIFI_EVENT_* in ESP-IDF. */
#define H_EVENT_WIFI_READY              0
#define H_EVENT_WIFI_SCAN_DONE          1
#define H_EVENT_WIFI_STA_START          2
#define H_EVENT_WIFI_STA_STOP           3
#define H_EVENT_WIFI_STA_CONNECTED      4
#define H_EVENT_WIFI_STA_DISCONNECTED   5
#define H_EVENT_WIFI_AP_START           6
#define H_EVENT_WIFI_AP_STOP            7
#define H_EVENT_WIFI_AP_STACONNECTED    8
#define H_EVENT_WIFI_AP_STADISCONNECTED 9
#define H_EVENT_WIFI_STA_BEACON_TIMEOUT 10
#define H_EVENT_WIFI_STA_AUTH_TIMEOUT   11
#define H_EVENT_WIFI_AUTHMODE_CHANGE    12
#define H_EVENT_WIFI_HOME_CHANNEL_CHANGE 13
#define H_EVENT_WIFI_ITWT_SETUP         14
#define H_EVENT_WIFI_ITWT_TEARDOWN      15
#define H_EVENT_WIFI_ITWT_SUSPEND       16
#define H_EVENT_WIFI_ITWT_PROBE         17
#define H_EVENT_WIFI_DPP_URI_READY      18
#define H_EVENT_WIFI_DPP_CFG_RECVD      19
#define H_EVENT_WIFI_DPP_FAILED         20

#ifndef H_DPP_URI_LEN_MAX
#define H_DPP_URI_LEN_MAX 0
#endif

#define H_EVENT_WIFI_DPP_URI_LEN_MAX (H_DPP_URI_LEN_MAX + 1)

/* ── IP Event IDs ── */
#define H_EVENT_IP_STA_GOT_IP           0
#define H_EVENT_IP_STA_LOST_IP          1
#define H_EVENT_IP_AP_STA_IP_ASSIGNED   2

/* ── Hosted Event IDs ── */
#define H_EVENT_HOSTED_CP_INIT          0
#define H_EVENT_HOSTED_CP_HEARTBEAT     1
#define H_EVENT_HOSTED_TRANSPORT_FAILURE 2
#define H_EVENT_HOSTED_TRANSPORT_UP     3
#define H_EVENT_HOSTED_TRANSPORT_DOWN   4
#define H_EVENT_HOSTED_MEM_MONITOR      5

/* Neutral Wi-Fi event payloads.
 * Core posts these portable structs. Ports translate them into native event
 * payloads expected by their event loop. */
typedef struct {
    uint8_t mac[6];
    uint8_t aid;
    bool is_mesh_child;
} h_event_wifi_ap_staconnected_t;

typedef struct {
    uint8_t mac[6];
    uint8_t aid;
    bool is_mesh_child;
    uint16_t reason;
} h_event_wifi_ap_stadisconnected_t;

typedef struct {
    uint32_t status;
    uint8_t number;
    uint8_t scan_id;
} h_event_wifi_sta_scan_done_t;

typedef struct {
    uint8_t ssid[32];
    uint8_t ssid_len;
    uint8_t bssid[6];
    uint8_t channel;
    h_wifi_auth_mode_t authmode;
    uint16_t aid;
} h_event_wifi_sta_connected_t;

typedef struct {
    uint8_t ssid[32];
    uint8_t ssid_len;
    uint8_t bssid[6];
    uint8_t reason;
    int8_t rssi;
} h_event_wifi_sta_disconnected_t;

typedef struct {
    uint16_t setup_cmd;
    uint8_t trigger;
    uint8_t flow_type;
    uint8_t flow_id;
    uint8_t wake_invl_expn;
    uint8_t wake_duration_unit;
    uint16_t reserved;
    uint8_t min_wake_dura;
    uint16_t wake_invl_mant;
    uint16_t twt_id;
    uint16_t timeout_time_ms;
} h_event_wifi_itwt_setup_config_t;

typedef struct {
    h_event_wifi_itwt_setup_config_t config;
    int32_t status;
    uint8_t reason;
    uint64_t target_wake_time;
} h_event_wifi_sta_itwt_setup_t;

typedef struct {
    uint8_t flow_id;
    int32_t status;
} h_event_wifi_sta_itwt_teardown_t;

typedef struct {
    int32_t status;
    uint8_t flow_id_bitmap;
    uint32_t actual_suspend_time_ms[8];
} h_event_wifi_sta_itwt_suspend_t;

typedef struct {
    int32_t status;
    uint8_t reason;
} h_event_wifi_sta_itwt_probe_t;

typedef struct {
    uint32_t uri_data_len;
    char uri[H_EVENT_WIFI_DPP_URI_LEN_MAX];
} h_event_wifi_dpp_uri_ready_t;

typedef struct {
    h_wifi_config_t wifi_cfg;
} h_event_wifi_dpp_config_received_t;

typedef struct {
    int failure_reason;
} h_event_wifi_dpp_failed_t;

/* ── Neutral Hosted Event Payloads ──
 * Core layer posts these portable structs through the event contract.
 * The port layer translates them into platform-native event types. */
typedef struct {
    uint32_t reason; /**< Reset reason from co-processor (neutral uint32_t) */
} h_event_hosted_init_t;

typedef struct {
    uint32_t heartbeat; /**< Current co-processor heartbeat number */
} h_event_hosted_heartbeat_t;

typedef struct {
    uint32_t free_size;
    uint32_t largest_free_block;
} h_event_hosted_mem_heap_t;

typedef struct {
    h_event_hosted_mem_heap_t cap_dma;
    h_event_hosted_mem_heap_t cap_8bit;
} h_event_hosted_mem_cap_info_t;

typedef struct {
    uint32_t curr_total_free_heap_size;
    uint32_t curr_min_free_heap_size;
    h_event_hosted_mem_cap_info_t curr_internal;
    h_event_hosted_mem_cap_info_t curr_external;
} h_event_hosted_mem_info_t;

/* ── Event Registration API (application-facing) ── */
#include "h_wrapper.h"  /* h_event_register / h_event_unregister */

#endif /* H_EVENT_H */
