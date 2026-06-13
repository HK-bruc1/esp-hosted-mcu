/* host/core/include/h_public/h_event.h */
#ifndef H_EVENT_H
#define H_EVENT_H

#include "h_types.h"
#include "h_wifi_types.h"

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
