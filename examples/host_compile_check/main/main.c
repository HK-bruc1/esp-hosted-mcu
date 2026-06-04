/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include "esp_log.h"
#include "esp_hosted_host_fw_ver.h"

static const char *TAG = "host_compile_check";

void app_main(void)
{
	uint32_t version = ESP_HOSTED_VERSION_VAL(ESP_HOSTED_VERSION_MAJOR_1,
			ESP_HOSTED_VERSION_MINOR_1,
			ESP_HOSTED_VERSION_PATCH_1);

	ESP_LOGI(TAG, "ESP-Hosted host compile check built, version "
			ESP_HOSTED_VERSION_PRINTF_FMT,
			ESP_HOSTED_VERSION_PRINTF_ARGS(version));
	ESP_LOGI(TAG, "No runtime transport initialization is performed");
}

