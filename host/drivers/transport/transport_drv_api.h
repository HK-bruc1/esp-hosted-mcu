/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TRANSPORT_DRV_API_H
#define TRANSPORT_DRV_API_H

/* Internal driver API — not a porting contract.
 * Each transport leaf driver (spi / sdio / spi_hd / uart) must provide
 * these symbols. Upper layers (core transport, port adapters) call them
 * via this header instead of bare extern declarations. */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  esp_hosted_tx(uint8_t iface_type, uint8_t iface_num,
                   uint8_t *payload_buf, uint16_t payload_len,
                   uint8_t buff_zerocopy, uint8_t *buffer_to_free,
                   void (*free_buf_func)(void *ptr), uint8_t flags);

void check_if_max_freq_used(uint8_t chip_type);

int  ensure_slave_bus_ready(void *bus_handle);

#ifdef __cplusplus
}
#endif

#endif /* TRANSPORT_DRV_API_H */
