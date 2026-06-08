/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_GPIO_H
#define H_TRANSPORT_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO helpers — ESP-IDF port internal API.
 * Use espidf_ prefix to avoid collision with h_wrapper.h macros
 * (h_gpio_config / h_gpio_read / etc. are #define'd to g_h_transport.*). */

int espidf_gpio_config(uint32_t gpio_num, uint32_t mode);
int espidf_gpio_setup_intr(uint32_t gpio_num, uint32_t intr_type,
                           void (*fn)(void *), void *arg);
int espidf_gpio_clear_intr(uint32_t gpio_num);
int espidf_gpio_read(uint32_t gpio_num);
int espidf_gpio_write(uint32_t gpio_num, uint32_t value);
int espidf_gpio_pull(uint32_t gpio_num, uint32_t pull_value, uint32_t enable);
int espidf_gpio_hold(uint32_t gpio_num, uint32_t hold_value);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_GPIO_H */
