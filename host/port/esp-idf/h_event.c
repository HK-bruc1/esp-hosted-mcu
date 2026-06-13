/* host/port/esp-idf/h_event.c
 * ESP-IDF Event port — maps h_event_contract_t to esp_event API.
 *
 * The portable h_event_base_t enum (H_EVENT_WIFI, H_EVENT_IP) is translated
 * to ESP-IDF event base strings (WIFI_EVENT, IP_EVENT). Handler signatures
 * are compatible — both use void(*)(void*, size_t, void*). */

#include "h_port_contract.h"
#include "h_event.h"
#include "h_wifi_type_adapt.h"

#include <freertos/FreeRTOS.h>  /* pdMS_TO_TICKS */
#include <esp_event.h>
#include <esp_wifi.h>           /* WIFI_EVENT */
#include <esp_netif.h>          /* IP_EVENT */
#include "esp_hosted_event.h"
#include "esp_hosted_misc_types.h"
#include <stdlib.h>
#include <string.h>

ESP_EVENT_DEFINE_BASE(ESP_HOSTED_EVENT);

/* ──  Helpers ── */

/* Map h_err_t <-> esp_err_t */
static h_err_t esp_err_to_h_err(esp_err_t err)
{
    switch (err) {
        case ESP_OK:              return H_OK;
        case ESP_ERR_NO_MEM:      return H_ERR_NO_MEM;
        case ESP_ERR_INVALID_ARG: return H_ERR_INVALID_ARG;
        case ESP_ERR_TIMEOUT:     return H_ERR_TIMEOUT;
        default:                  return H_FAIL;
    }
}

/* Map portable event base to ESP-IDF event base string */
static esp_event_base_t h_base_to_esp(h_event_base_t base)
{
    switch (base) {
        case H_EVENT_WIFI:   return WIFI_EVENT;
        case H_EVENT_IP:     return IP_EVENT;
        case H_EVENT_HOSTED: return ESP_HOSTED_EVENT;
        default:             return NULL;
    }
}

static int32_t h_wifi_event_id_to_esp(int32_t event_id)
{
    switch (event_id) {
        case H_EVENT_WIFI_READY:              return WIFI_EVENT_WIFI_READY;
        case H_EVENT_WIFI_SCAN_DONE:          return WIFI_EVENT_SCAN_DONE;
        case H_EVENT_WIFI_STA_START:          return WIFI_EVENT_STA_START;
        case H_EVENT_WIFI_STA_STOP:           return WIFI_EVENT_STA_STOP;
        case H_EVENT_WIFI_STA_CONNECTED:      return WIFI_EVENT_STA_CONNECTED;
        case H_EVENT_WIFI_STA_DISCONNECTED:   return WIFI_EVENT_STA_DISCONNECTED;
        case H_EVENT_WIFI_AUTHMODE_CHANGE:    return WIFI_EVENT_STA_AUTHMODE_CHANGE;
        case H_EVENT_WIFI_AP_START:           return WIFI_EVENT_AP_START;
        case H_EVENT_WIFI_AP_STOP:            return WIFI_EVENT_AP_STOP;
        case H_EVENT_WIFI_AP_STACONNECTED:    return WIFI_EVENT_AP_STACONNECTED;
        case H_EVENT_WIFI_AP_STADISCONNECTED: return WIFI_EVENT_AP_STADISCONNECTED;
        case H_EVENT_WIFI_HOME_CHANNEL_CHANGE:return WIFI_EVENT_HOME_CHANNEL_CHANGE;
#if H_WIFI_HE_SUPPORT
        case H_EVENT_WIFI_ITWT_SETUP:         return WIFI_EVENT_ITWT_SETUP;
        case H_EVENT_WIFI_ITWT_TEARDOWN:      return WIFI_EVENT_ITWT_TEARDOWN;
        case H_EVENT_WIFI_ITWT_SUSPEND:       return WIFI_EVENT_ITWT_SUSPEND;
        case H_EVENT_WIFI_ITWT_PROBE:         return WIFI_EVENT_ITWT_PROBE;
#endif
#if H_WIFI_DPP_SUPPORT
        case H_EVENT_WIFI_DPP_URI_READY:      return WIFI_EVENT_DPP_URI_READY;
        case H_EVENT_WIFI_DPP_CFG_RECVD:      return WIFI_EVENT_DPP_CFG_RECVD;
        case H_EVENT_WIFI_DPP_FAILED:         return WIFI_EVENT_DPP_FAILED;
#endif
        default:                              return -1;
    }
}

static h_err_t post_no_payload_wifi_event(int32_t event_id, void *event_data,
                                          size_t event_data_size,
                                          TickType_t ticks)
{
    if (event_data || event_data_size != 0) {
        return H_ERR_INVALID_ARG;
    }
    return esp_err_to_h_err(esp_event_post(WIFI_EVENT, event_id, NULL, 0, ticks));
}

static h_err_t h_event_post_wifi_adapter(int32_t event_id, void *event_data,
                                         size_t event_data_size,
                                         TickType_t ticks)
{
    int32_t esp_event_id = h_wifi_event_id_to_esp(event_id);
    if (esp_event_id < 0) {
        return H_ERR_INVALID_ARG;
    }

    switch (event_id) {
        case H_EVENT_WIFI_READY:
        case H_EVENT_WIFI_STA_START:
        case H_EVENT_WIFI_STA_STOP:
        case H_EVENT_WIFI_AP_START:
        case H_EVENT_WIFI_AP_STOP:
        case H_EVENT_WIFI_AUTHMODE_CHANGE:
        case H_EVENT_WIFI_HOME_CHANNEL_CHANGE:
            return post_no_payload_wifi_event(esp_event_id, event_data,
                                              event_data_size, ticks);

        case H_EVENT_WIFI_AP_STACONNECTED: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_ap_staconnected_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_ap_staconnected_t *src =
                (h_event_wifi_ap_staconnected_t *)event_data;
            wifi_event_ap_staconnected_t dst = { 0 };
            memcpy(dst.mac, src->mac, sizeof(dst.mac));
            dst.aid = src->aid;
            dst.is_mesh_child = src->is_mesh_child;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_AP_STADISCONNECTED: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_ap_stadisconnected_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_ap_stadisconnected_t *src =
                (h_event_wifi_ap_stadisconnected_t *)event_data;
            wifi_event_ap_stadisconnected_t dst = { 0 };
            memcpy(dst.mac, src->mac, sizeof(dst.mac));
            dst.aid = src->aid;
            dst.is_mesh_child = src->is_mesh_child;
            dst.reason = src->reason;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_SCAN_DONE: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_scan_done_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_scan_done_t *src =
                (h_event_wifi_sta_scan_done_t *)event_data;
            wifi_event_sta_scan_done_t dst = { 0 };
            dst.status = src->status;
            dst.number = src->number;
            dst.scan_id = src->scan_id;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_STA_CONNECTED: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_connected_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_connected_t *src =
                (h_event_wifi_sta_connected_t *)event_data;
            wifi_event_sta_connected_t dst = { 0 };
            memcpy(dst.ssid, src->ssid, sizeof(dst.ssid));
            dst.ssid_len = src->ssid_len;
            memcpy(dst.bssid, src->bssid, sizeof(dst.bssid));
            dst.channel = src->channel;
            dst.authmode = h_wifi_adapt_auth_to_native(src->authmode);
            dst.aid = src->aid;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_STA_DISCONNECTED: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_disconnected_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_disconnected_t *src =
                (h_event_wifi_sta_disconnected_t *)event_data;
            wifi_event_sta_disconnected_t dst = { 0 };
            memcpy(dst.ssid, src->ssid, sizeof(dst.ssid));
            dst.ssid_len = src->ssid_len;
            memcpy(dst.bssid, src->bssid, sizeof(dst.bssid));
            dst.reason = src->reason;
            dst.rssi = src->rssi;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

#if H_WIFI_HE_SUPPORT
        case H_EVENT_WIFI_ITWT_SETUP: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_itwt_setup_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_itwt_setup_t *src =
                (h_event_wifi_sta_itwt_setup_t *)event_data;
            wifi_event_sta_itwt_setup_t dst = { 0 };
            dst.config.setup_cmd = src->config.setup_cmd;
            dst.config.trigger = src->config.trigger;
            dst.config.flow_type = src->config.flow_type;
            dst.config.flow_id = src->config.flow_id;
            dst.config.wake_invl_expn = src->config.wake_invl_expn;
            dst.config.wake_duration_unit = src->config.wake_duration_unit;
#if H_DECODE_WIFI_RESERVED_FIELD
            dst.config.reserved = src->config.reserved;
#endif
            dst.config.min_wake_dura = src->config.min_wake_dura;
            dst.config.wake_invl_mant = src->config.wake_invl_mant;
            dst.config.twt_id = src->config.twt_id;
            dst.config.timeout_time_ms = src->config.timeout_time_ms;
            dst.status = src->status;
            dst.reason = src->reason;
            dst.target_wake_time = src->target_wake_time;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_ITWT_TEARDOWN: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_itwt_teardown_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_itwt_teardown_t *src =
                (h_event_wifi_sta_itwt_teardown_t *)event_data;
            wifi_event_sta_itwt_teardown_t dst = { 0 };
            dst.flow_id = src->flow_id;
            dst.status = src->status;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_ITWT_SUSPEND: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_itwt_suspend_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_itwt_suspend_t *src =
                (h_event_wifi_sta_itwt_suspend_t *)event_data;
            wifi_event_sta_itwt_suspend_t dst = { 0 };
            dst.status = src->status;
            dst.flow_id_bitmap = src->flow_id_bitmap;
            memcpy(dst.actual_suspend_time_ms, src->actual_suspend_time_ms,
                   sizeof(dst.actual_suspend_time_ms));
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_ITWT_PROBE: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_sta_itwt_probe_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_sta_itwt_probe_t *src =
                (h_event_wifi_sta_itwt_probe_t *)event_data;
            wifi_event_sta_itwt_probe_t dst = { 0 };
            dst.status = src->status;
            dst.reason = src->reason;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }
#endif

#if H_WIFI_DPP_SUPPORT
        case H_EVENT_WIFI_DPP_URI_READY: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_dpp_uri_ready_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_dpp_uri_ready_t *src =
                (h_event_wifi_dpp_uri_ready_t *)event_data;
            if (src->uri_data_len > H_EVENT_WIFI_DPP_URI_LEN_MAX) {
                return H_ERR_INVALID_ARG;
            }
            size_t native_size = sizeof(wifi_event_dpp_uri_ready_t) +
                                 src->uri_data_len;
            wifi_event_dpp_uri_ready_t *dst = calloc(1, native_size);
            if (!dst) {
                return H_ERR_NO_MEM;
            }
            dst->uri_data_len = src->uri_data_len;
            if (src->uri_data_len > 0) {
                memcpy(dst->uri, src->uri, src->uri_data_len);
            }
            h_err_t ret = esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, dst, native_size, ticks));
            free(dst);
            return ret;
        }

        case H_EVENT_WIFI_DPP_CFG_RECVD: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_dpp_config_received_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_dpp_config_received_t *src =
                (h_event_wifi_dpp_config_received_t *)event_data;
            wifi_event_dpp_config_received_t dst = { 0 };
            h_wifi_adapt_config_to_native(&src->wifi_cfg, &dst.wifi_cfg);
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }

        case H_EVENT_WIFI_DPP_FAILED: {
            if (!event_data ||
                event_data_size != sizeof(h_event_wifi_dpp_failed_t)) {
                return H_ERR_INVALID_ARG;
            }
            h_event_wifi_dpp_failed_t *src =
                (h_event_wifi_dpp_failed_t *)event_data;
            wifi_event_dpp_failed_t dst = { 0 };
            dst.failure_reason = src->failure_reason;
            return esp_err_to_h_err(esp_event_post(
                WIFI_EVENT, esp_event_id, &dst, sizeof(dst), ticks));
        }
#endif

        default:
            return H_ERR_INVALID_ARG;
    }
}

/* ──  Event Adapters ── */

static int h_event_register_adapter(h_event_base_t base, int32_t event_id,
                                    h_event_handler_t handler, void *user_ctx)
{
    esp_event_base_t esp_base = h_base_to_esp(base);
    if (!esp_base) return H_ERR_INVALID_ARG;

    if (base == H_EVENT_WIFI) {
        event_id = h_wifi_event_id_to_esp(event_id);
        if (event_id < 0) return H_ERR_INVALID_ARG;
    }

    /* NOTE: Cast from h_event_handler_t(void *data, size_t len, void *ctx)
     * to esp_event_handler_t(void *arg, base, id, void *data) is formally UB
     * per C11 6.3.2.3p8. Safe on Xtensa/RISC-V ABI — extra params passed in
     * unused registers. Phase 2 will add a proper trampoline adapter. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    esp_err_t ret = esp_event_handler_register(
        esp_base, event_id,
        (esp_event_handler_t)handler, user_ctx);
#pragma GCC diagnostic pop

    return esp_err_to_h_err(ret);
}

static int h_event_unregister_adapter(h_event_base_t base, int32_t event_id,
                                      h_event_handler_t handler)
{
    esp_event_base_t esp_base = h_base_to_esp(base);
    if (!esp_base) return H_ERR_INVALID_ARG;

    if (base == H_EVENT_WIFI) {
        event_id = h_wifi_event_id_to_esp(event_id);
        if (event_id < 0) return H_ERR_INVALID_ARG;
    }

    /* NOTE: Cast from h_event_handler_t(void *data, size_t len, void *ctx)
     * to esp_event_handler_t(void *arg, base, id, void *data) is formally UB
     * per C11 6.3.2.3p8. Safe on Xtensa/RISC-V ABI — extra params passed in
     * unused registers. Phase 2 will add a proper trampoline adapter. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    esp_err_t ret = esp_event_handler_unregister(
        esp_base, event_id,
        (esp_event_handler_t)handler);
#pragma GCC diagnostic pop

    return esp_err_to_h_err(ret);
}

static int h_event_post_adapter(h_event_base_t base, int32_t event_id,
                                void *event_data, size_t event_data_size)
{
    esp_event_base_t esp_base = h_base_to_esp(base);
    if (!esp_base) return H_ERR_INVALID_ARG;

    if (base == H_EVENT_WIFI) {
        return h_event_post_wifi_adapter(event_id, event_data,
                                         event_data_size, portMAX_DELAY);
    }

    /* If the event is a hosted event, translate neutral payload to
     * ESP-IDF native payload before posting. */
    if (base == H_EVENT_HOSTED) {
        switch (event_id) {
            case H_EVENT_HOSTED_CP_INIT: {
                if (event_data && event_data_size == sizeof(h_event_hosted_init_t)) {
                    h_event_hosted_init_t *src = (h_event_hosted_init_t *)event_data;
                    /* The ESP-IDF port stores esp_reset_reason_t's numeric
                     * value in the neutral payload. Other ports must map this
                     * value explicitly to their own reset-reason type. */
                    esp_hosted_event_init_t dst = { .reason = (esp_reset_reason_t)src->reason };
                    return esp_err_to_h_err(esp_event_post(
                        esp_base, ESP_HOSTED_EVENT_CP_INIT,
                        &dst, sizeof(dst), portMAX_DELAY));
                }
                break;
            }
            case H_EVENT_HOSTED_CP_HEARTBEAT: {
                if (event_data && event_data_size == sizeof(h_event_hosted_heartbeat_t)) {
                    h_event_hosted_heartbeat_t *src = (h_event_hosted_heartbeat_t *)event_data;
                    esp_hosted_event_heartbeat_t dst = { .heartbeat = src->heartbeat };
                    return esp_err_to_h_err(esp_event_post(
                        esp_base, ESP_HOSTED_EVENT_CP_HEARTBEAT,
                        &dst, sizeof(dst), portMAX_DELAY));
                }
                break;
            }
            case H_EVENT_HOSTED_MEM_MONITOR: {
                if (event_data && event_data_size == sizeof(h_event_hosted_mem_info_t)) {
                    h_event_hosted_mem_info_t *src = (h_event_hosted_mem_info_t *)event_data;
                    esp_hosted_event_mem_info_t dst = { 0 };

                    dst.curr_total_free_heap_size = src->curr_total_free_heap_size;
                    dst.curr_min_free_heap_size = src->curr_min_free_heap_size;
                    dst.curr_internal.cap_dma.free_size =
                        src->curr_internal.cap_dma.free_size;
                    dst.curr_internal.cap_dma.largest_free_block =
                        src->curr_internal.cap_dma.largest_free_block;
                    dst.curr_internal.cap_8bit.free_size =
                        src->curr_internal.cap_8bit.free_size;
                    dst.curr_internal.cap_8bit.largest_free_block =
                        src->curr_internal.cap_8bit.largest_free_block;
                    dst.curr_external.cap_dma.free_size =
                        src->curr_external.cap_dma.free_size;
                    dst.curr_external.cap_dma.largest_free_block =
                        src->curr_external.cap_dma.largest_free_block;
                    dst.curr_external.cap_8bit.free_size =
                        src->curr_external.cap_8bit.free_size;
                    dst.curr_external.cap_8bit.largest_free_block =
                        src->curr_external.cap_8bit.largest_free_block;

                    return esp_err_to_h_err(esp_event_post(
                        esp_base, ESP_HOSTED_EVENT_MEM_MONITOR,
                        &dst, sizeof(dst), portMAX_DELAY));
                }
                break;
            }
            case H_EVENT_HOSTED_TRANSPORT_UP: {
                if (event_data || event_data_size != 0) {
                    break;
                }
                return esp_err_to_h_err(esp_event_post(
                    esp_base, ESP_HOSTED_EVENT_TRANSPORT_UP,
                    NULL, 0, portMAX_DELAY));
            }
            case H_EVENT_HOSTED_TRANSPORT_DOWN: {
                if (event_data || event_data_size != 0) {
                    break;
                }
                return esp_err_to_h_err(esp_event_post(
                    esp_base, ESP_HOSTED_EVENT_TRANSPORT_DOWN,
                    NULL, 0, portMAX_DELAY));
            }
            case H_EVENT_HOSTED_TRANSPORT_FAILURE: {
                if (event_data || event_data_size != 0) {
                    break;
                }
                return esp_err_to_h_err(esp_event_post(
                    esp_base, ESP_HOSTED_EVENT_TRANSPORT_FAILURE,
                    NULL, 0, portMAX_DELAY));
            }
            default:
                break;
        }

        return H_ERR_INVALID_ARG;
    }

    esp_err_t ret = esp_event_post(
        esp_base, event_id,
        event_data, event_data_size,
        portMAX_DELAY);

    return esp_err_to_h_err(ret);
}

static int h_event_wifi_post_adapter(int32_t event_id, void *event_data,
                                     size_t event_data_size,
                                     int32_t timeout_ms)
{
    TickType_t ticks = (timeout_ms < 0)
                           ? portMAX_DELAY
                           : pdMS_TO_TICKS((uint32_t)timeout_ms);

    return h_event_post_wifi_adapter(event_id, event_data, event_data_size, ticks);
}

/* ──  Global Event Contract Instance ── */

const h_event_contract_t g_h_event = {
    .register_handler   = h_event_register_adapter,
    .unregister_handler = h_event_unregister_adapter,
    .post               = h_event_post_adapter,
    .wifi_post          = h_event_wifi_post_adapter,
};
