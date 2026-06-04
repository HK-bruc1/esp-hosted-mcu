/* host/port/esp-idf/h_control_serial_adapter.h
 *
 * Control Serial Adapter — ESP-IDF port implementation.
 *
 * This is a Path B adapter (not a contract): the core layer calls these
 * symbols via extern declarations. When a second-platform PoC restarts,
 * this adapter is the starting point for extracting h_control_contract_t.
 *
 * Symbols use the h_control_serial_* prefix to make the adapter boundary
 * explicit and distinguishable from the old serial_drv_* / transport_pserial_*
 * legacy interfaces. */
#ifndef H_CONTROL_SERIAL_ADAPTER_H
#define H_CONTROL_SERIAL_ADAPTER_H

#include <stdint.h>

/* Opaque handle — callers only pass the pointer through */
struct h_control_serial_handle;
typedef struct h_control_serial_handle h_control_serial_handle_t;

h_control_serial_handle_t *h_control_serial_drv_open (const char *transport);
int h_control_serial_drv_close(h_control_serial_handle_t **handle);
int h_control_serial_drv_write(h_control_serial_handle_t *handle,
                                uint8_t *buf, int in_count, int *out_count);
uint8_t *h_control_serial_drv_read (h_control_serial_handle_t *handle,
                                     uint32_t *out_nbyte);
int h_control_serial_platform_init  (void);
int h_control_serial_platform_deinit(void);

#endif /* H_CONTROL_SERIAL_ADAPTER_H */
