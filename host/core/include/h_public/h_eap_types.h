/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_EAP_TYPES_H
#define H_EAP_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef int32_t h_eap_ttls_phase2_types_t;
typedef int32_t h_eap_method_t;

typedef struct {
	int32_t fast_provisioning;
	int32_t fast_max_pac_list_len;
	bool fast_pac_format_binary;
} h_eap_fast_config_t;

#endif /* H_EAP_TYPES_H */
