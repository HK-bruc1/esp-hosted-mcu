/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "esp_log.h"

/* Core portable types + alignment constants */
#include "h_types.h"
#include "h_wifi_types.h"

/* Port contract + wrappers */
#include "h_port_contract.h"
#include "h_wrapper.h"

/* Port config (task defaults, GPIO, transport buffer, power-save Kconfig) */
#include "h_port_config.h"

/* Host version */
#include "esp_hosted_host_fw_ver.h"

static const char *TAG = "host_compile_check";

/* Compile-time verification:
 * Include the headers above is sufficient — the component CMakeLists compiles
 * all active source files. This function references key symbols to catch
 * missing declarations, type mismatches, or macro breakage. */
static void verify_contract_compile(void)
{
    /* h_types.h — error codes */
    h_err_t err = H_OK;
    (void)err;

    /* h_types.h — alignment constants */
    uint32_t align = H_MEM_ALIGNMENT_64;
    (void)align;

    /* h_port_config.h — task defaults */
    uint32_t prio = H_DEFAULT_TASK_PRIO;
    uint32_t stack = H_DEFAULT_TASK_STACK;
    uint32_t rpc_stack = H_DEFAULT_RPC_TASK_STACK;
    (void)prio;
    (void)stack;
    (void)rpc_stack;

    /* h_port_config.h — GPIO constants */
    int gpio_mode = H_GPIO_MODE_INPUT;
    int pull_up = H_GPIO_PULL_UP;
    int pull_down = H_GPIO_PULL_DOWN;
    int gpio_enable = H_ENABLE;
    (void)gpio_mode;
    (void)pull_up;
    (void)pull_down;
    (void)gpio_enable;

    /* h_port_config.h — transport buffer */
    uint32_t buf_size = H_MAX_TRANSPORT_BUFFER_SIZE;
    (void)buf_size;

    /* h_port_contract.h — contract struct types */
    const h_osal_contract_t *osal = &g_h_osal;
    const h_event_contract_t *evt = &g_h_event;
    const h_transport_contract_t *tr = &g_h_transport;
    (void)osal;
    (void)evt;
    (void)tr;

    /* h_wrapper.h — wrapper macro expansion check (compile-only, no runtime call) */
    if (0) {
        (void)h_malloc(64);
        (void)h_calloc(1, 64);
        h_free(NULL);
        (void)h_get_time_ms();
        h_msleep(0);
        (void)h_gpio_config(0, 0);
        (void)h_gpio_read(0);
        (void)h_gpio_write(0, 0);
        (void)h_gpio_pull(0, 0, false);
        (void)h_gpio_hold(0, false);
        (void)h_restart_host();
        (void)h_ps_init();
        (void)h_woke_from_ps();
        (void)h_get_host_wakeup_or_reboot_reason();
        (void)h_config_host_power_save_hal(0, 0, 0);
        (void)h_start_host_power_save_hal(0);
    }
}

void app_main(void)
{
    uint32_t version = ESP_HOSTED_VERSION_VAL(ESP_HOSTED_VERSION_MAJOR_1,
            ESP_HOSTED_VERSION_MINOR_1,
            ESP_HOSTED_VERSION_PATCH_1);

    ESP_LOGI(TAG, "ESP-Hosted host compile check built, version "
            ESP_HOSTED_VERSION_PRINTF_FMT,
            ESP_HOSTED_VERSION_PRINTF_ARGS(version));
    ESP_LOGI(TAG, "No runtime transport initialization is performed");

    verify_contract_compile();

    ESP_LOGI(TAG, "All contract/wrapper headers compile OK");
}
