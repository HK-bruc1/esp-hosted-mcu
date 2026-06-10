/* host/port/include/h_control_serial_contract.h
 *
 * Control Serial Adapter — platform-neutral contract.
 *
 * Declares the function signatures that the control serial adapter must
 * implement. Port layers provide the actual implementation (e.g.
 * host/port/esp-idf/h_control_serial_adapter.c).
 *
 * Core layer and driver layer include THIS header, not the port-specific
 * adapter header. */

#ifndef H_CONTROL_SERIAL_CONTRACT_H
#define H_CONTROL_SERIAL_CONTRACT_H

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

#endif /* H_CONTROL_SERIAL_CONTRACT_H */
