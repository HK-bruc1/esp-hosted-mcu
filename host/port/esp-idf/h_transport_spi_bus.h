/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_SPI_BUS_H
#define H_TRANSPORT_SPI_BUS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SPI bus helpers — ESP-IDF port internal API.
 * Replaces legacy hosted_spi_*() from port_esp_hosted_host_spi.c. */

void *h_spi_bus_init(void);
int   h_spi_bus_deinit(void *handle);
int   h_spi_bus_transfer(void *trans);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_SPI_BUS_H */
