/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_UART_BUS_H
#define H_TRANSPORT_UART_BUS_H

#include <stdint.h>
#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UART bus helpers — ESP-IDF port internal API.
 * Replaces legacy hosted_uart_*() from port_esp_hosted_host_uart.c. */

void     *h_uart_bus_init(void);
esp_err_t h_uart_bus_deinit(void *ctx);
int       h_uart_bus_read(void *ctx, uint8_t *data, uint16_t size);
int       h_uart_bus_write(void *ctx, uint8_t *data, uint16_t size);
int       h_uart_bus_flush_input(void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_UART_BUS_H */
