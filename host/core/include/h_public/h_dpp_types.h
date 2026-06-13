/*
 * ESP-Hosted Host Core — Platform-Independent DPP Types
 *
 * Portable replacements for ESP-IDF esp_supp_dpp_event_t,
 * esp_supp_dpp_event_cb_t, and esp_supp_dpp_bootstrap_t.
 * Port layers must provide static_assert or adapter to verify
 * value compatibility with the platform's native DPP types.
 */

#ifndef H_DPP_TYPES_H
#define H_DPP_TYPES_H

#include <stdint.h>

typedef enum {
	H_SUPP_DPP_EVENT_URI_READY = 0,
	H_SUPP_DPP_EVENT_CFG_RECVD = 1,
	H_SUPP_DPP_EVENT_FAIL      = 2,
} h_supp_dpp_event_t;

typedef void (*h_supp_dpp_event_cb_t)(h_supp_dpp_event_t evt, void *data);

typedef enum {
	H_SUPP_DPP_BOOTSTRAP_QR_CODE  = 1,
	H_SUPP_DPP_BOOTSTRAP_PKEX     = 2,
} h_supp_dpp_bootstrap_t;

#endif /* H_DPP_TYPES_H */
