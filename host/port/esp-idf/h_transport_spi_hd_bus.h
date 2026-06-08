/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_SPI_HD_BUS_H
#define H_TRANSPORT_SPI_HD_BUS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SPI-HD bus helpers — ESP-IDF port internal API.
 * Replaces legacy hosted_spi_hd_*() from port_esp_hosted_host_spi_hd.c. */

void *h_spi_hd_bus_init(void);
int   h_spi_hd_bus_deinit(void *ctx);
int   h_spi_hd_bus_read_reg(uint32_t reg, uint32_t *data, int poll, bool lock_required);
int   h_spi_hd_bus_write_reg(uint32_t reg, uint32_t *data, bool lock_required);
int   h_spi_hd_bus_read_dma(uint8_t *data, uint16_t size, bool lock_required);
int   h_spi_hd_bus_write_dma(uint8_t *data, uint16_t size, bool lock_required);
int   h_spi_hd_bus_send_cmd9(void);

/* Weak/strong exception — keeps legacy name.
 * h_osal.c provides __attribute__((weak)) stub returning H_ERR_NOT_SUP.
 * This file provides the strong definition when SPI-HD is the active transport. */
int   hosted_spi_hd_set_data_lines(uint32_t data_lines);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_SPI_HD_BUS_H */
