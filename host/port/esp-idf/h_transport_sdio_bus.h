/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_SDIO_BUS_H
#define H_TRANSPORT_SDIO_BUS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SDIO bus helpers — ESP-IDF port internal API.
 * Replaces legacy hosted_sdio_*() from port_esp_hosted_host_sdio.c. */

void *h_sdio_bus_init(void);
int   h_sdio_bus_deinit(void *ctx);
int   h_sdio_bus_card_init(void *ctx, bool show_config);
int   h_sdio_bus_card_deinit(void *ctx);
int   h_sdio_bus_read_reg(void *ctx, uint32_t reg, uint8_t *data, uint16_t size, bool lock_required);
int   h_sdio_bus_write_reg(void *ctx, uint32_t reg, uint8_t *data, uint16_t size, bool lock_required);
int   h_sdio_bus_read_block(void *ctx, uint32_t reg, uint8_t *data, uint16_t size, bool lock_required);
int   h_sdio_bus_write_block(void *ctx, uint32_t reg, uint8_t *data, uint16_t size, bool lock_required);
int   h_sdio_bus_wait_intr(void *ctx, uint32_t ticks_to_wait);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_SDIO_BUS_H */
