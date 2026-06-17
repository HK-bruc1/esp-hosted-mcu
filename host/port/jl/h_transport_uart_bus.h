/* host/port/jl/h_transport_uart_bus.h
 */

#ifndef H_TRANSPORT_UART_BUS_JL_H
#define H_TRANSPORT_UART_BUS_JL_H

#include <stdint.h>

void *jl_uart_bus_init(void);
int  jl_uart_bus_deinit(void *handle);
int  jl_uart_bus_read(void *handle, uint8_t *data, uint16_t size);
int  jl_uart_bus_write(void *handle, uint8_t *data, uint16_t size);
int  jl_uart_bus_flush_input(void *handle);

void *jl_uart_get_handle(void);
void  jl_uart_transport_start(void);
void  jl_uart_transport_stop(void);

#endif /* H_TRANSPORT_UART_BUS_JL_H */
