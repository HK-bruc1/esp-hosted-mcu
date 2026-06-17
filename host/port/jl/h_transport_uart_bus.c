/* host/port/jl/h_transport_uart_bus.c
 * JL UART HAL adapter using uart_v1 driver.
 */

#include "h_port_config.h"
#include "h_types.h"
#include "system/malloc.h"
#include "driver/cpu/uart_v1.h"

typedef struct {
    int uart_num;
} jl_uart_bus_t;

void *jl_uart_bus_init(void)
{
    struct uart_config cfg = {
        .baud_rate = H_UART_BAUD_RATE,
        .tx_pin    = H_UART_PIN_TX,
        .rx_pin    = H_UART_PIN_RX,
        .parity    = UART_PARITY_DISABLE,
    };

    s32 num = uart_init_new(H_UART_PORT, &cfg);
    if (num < 0) {
        return NULL;
    }

    jl_uart_bus_t *bus = (jl_uart_bus_t *)malloc(sizeof(jl_uart_bus_t));
    if (!bus) {
        uart_deinit((uart_dev)num);
        return NULL;
    }
    bus->uart_num = (int)num;
    return bus;
}

int jl_uart_bus_deinit(void *handle)
{
    if (!handle) {
        return H_ERR_INVALID_ARG;
    }
    jl_uart_bus_t *bus = (jl_uart_bus_t *)handle;
    uart_deinit((uart_dev)bus->uart_num);
    free(bus);
    return H_OK;
}

int jl_uart_bus_read(void *handle, uint8_t *data, uint16_t size)
{
    if (!handle || !data || !size) {
        return H_ERR_INVALID_ARG;
    }
    jl_uart_bus_t *bus = (jl_uart_bus_t *)handle;
    s32 ret = uart_recv_bytes((uart_dev)bus->uart_num, data, size);
    if (ret < 0) {
        return H_FAIL;
    }
    /* Return 0 unchanged so the caller can distinguish "no data yet"
     * from a real driver error. */
    return (int)ret;
}

int jl_uart_bus_write(void *handle, uint8_t *data, uint16_t size)
{
    if (!handle || !data || !size) {
        return H_ERR_INVALID_ARG;
    }
    jl_uart_bus_t *bus = (jl_uart_bus_t *)handle;
    s32 ret = uart_send_blocking((uart_dev)bus->uart_num, data, size, 0);
    if (ret < 0) {
        return H_FAIL;
    }
    return (int)ret;
}

int jl_uart_bus_flush_input(void *handle)
{
    if (!handle) {
        return H_ERR_INVALID_ARG;
    }
    jl_uart_bus_t *bus = (jl_uart_bus_t *)handle;
    uint8_t tmp[32];
    while (uart_recv_bytes((uart_dev)bus->uart_num, tmp, sizeof(tmp)) > 0) {
        /* drain */
    }
    return H_OK;
}
