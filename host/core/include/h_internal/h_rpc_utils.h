/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __H_RPC_UTILS_H
#define __H_RPC_UTILS_H

#include "h_types.h"
#include "h_wifi_types.h"
#include "esp_hosted_rpc.pb-c.h"

h_err_t rpc_copy_wifi_sta_config(h_wifi_config_t *dst, WifiStaConfig *src);

#endif
