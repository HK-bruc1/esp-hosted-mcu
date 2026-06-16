/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "h_wrapper.h"
#include "h_rpc_utils.h"
#include "h_rpc_core.h"

#include "h_config.h"
#include "esp_hosted_bitmasks.h"

#define RPC_UTILS_COPY_BYTES(dst, src) {                                      \
    if ((src).data && (src).len) {                                            \
        h_memcpy((dst), (src).data, H_MIN((src).len, sizeof(dst)));           \
    }                                                                        \
}

h_err_t rpc_copy_wifi_sta_config(h_wifi_config_t *dst, WifiStaConfig *src)
{
	if (!dst || !src) {
		return H_FAIL;
	}

	h_memset(dst, 0, sizeof(*dst));

	RPC_UTILS_COPY_BYTES(dst->sta.ssid, src->ssid);
	dst->sta.ssid_len = H_MIN(src->ssid.len, sizeof(dst->sta.ssid));
	RPC_UTILS_COPY_BYTES(dst->sta.password, src->password);
	RPC_UTILS_COPY_BYTES(dst->sta.bssid, src->bssid);
	dst->sta.channel = src->channel;
	dst->sta.listen_interval = src->listen_interval;
	if (src->pmf_cfg) {
		dst->sta.pmf_cfg_capable = src->pmf_cfg->capable ? 1 : 0;
		dst->sta.pmf_cfg_required = src->pmf_cfg->required ? 1 : 0;
	}

	return H_FAIL;
}
