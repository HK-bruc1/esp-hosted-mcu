/* host/core/include/h_internal/h_serial_if.h */
#ifndef H_SERIAL_IF_H
#define H_SERIAL_IF_H

#include "h_types.h"
#include <stdint.h>

/* Serial interface abstraction for control path communication.
 * Routes protobuf-encoded RPC messages over the transport layer. */

#define H_SERIAL_TLV_T_EPNAME           0x01
#define H_SERIAL_TLV_T_DATA             0x02

#define H_SERIAL_SIZE_OF_TYPE           1
#define H_SERIAL_SIZE_OF_LENGTH         2

h_err_t h_serial_if_init(void);
void    h_serial_if_deinit(void);
h_err_t h_serial_if_send(const uint8_t *data, uint16_t len);
h_err_t h_serial_if_recv(uint8_t *data, uint16_t *len, int32_t timeout_ms);
bool    h_serial_if_is_ready(void);

#endif
