/* host/port/jl/h_transport_uart.c
 * JL UART transport contract.
 */

#include "h_port_contract.h"
#include "h_port_config.h"
#include "h_types.h"
#include "h_transport_uart_bus.h"
#include "transport_drv_api.h"
#include "system/generic/gpio.h"

#if H_TRANSPORT_IN_USE == H_TRANSPORT_UART

static void *g_uart_handle = NULL;

static int h_uart_init_adapter(void **out_handle)
{
    if (!out_handle) {
        return H_ERR_INVALID_ARG;
    }
    if (!g_uart_handle) {
        g_uart_handle = jl_uart_bus_init();
    }
    *out_handle = g_uart_handle;
    return (g_uart_handle != NULL) ? H_OK : H_FAIL;
}

static int h_uart_deinit_adapter(void *handle)
{
    if (!handle) {
        return H_ERR_INVALID_ARG;
    }
    jl_uart_transport_stop();
    int ret = jl_uart_bus_deinit(g_uart_handle);
    g_uart_handle = NULL;
    return ret;
}

static int h_uart_read_adapter(void *handle, uint8_t *data, uint16_t size)
{
    if (!handle || !data || !size) {
        return H_ERR_INVALID_ARG;
    }
    int ret = jl_uart_bus_read(handle, data, size);
    return (ret >= 0) ? ret : H_FAIL;
}

static int h_uart_write_adapter(void *handle, uint8_t *data, uint16_t size)
{
    if (!handle || !data || !size) {
        return H_ERR_INVALID_ARG;
    }
    int ret = jl_uart_bus_write(handle, data, size);
    return (ret >= 0) ? ret : H_FAIL;
}

static int h_uart_flush_adapter(void *handle)
{
    if (!handle) {
        return H_ERR_INVALID_ARG;
    }
    return jl_uart_bus_flush_input(handle);
}

/* GPIO */
static int h_gpio_config_adapter(uint32_t pin, uint32_t mode)
{
    if (mode == H_GPIO_MODE_OUTPUT) {
        gpio_direction_output(pin, 0);
    } else {
        gpio_direction_input(pin);
    }
    return H_OK;
}

static int h_gpio_set_intr_adapter(uint32_t pin, uint32_t intr_type,
                                   void (*isr)(void*), void *arg)
{
    (void)pin;
    (void)intr_type;
    (void)isr;
    (void)arg;
    return H_ERR_NOT_SUP;
}

static int h_gpio_clear_intr_adapter(uint32_t pin)
{
    (void)pin;
    return H_ERR_NOT_SUP;
}

static int h_gpio_read_adapter(uint32_t pin)
{
    return gpio_read(pin);
}

static int h_gpio_write_adapter(uint32_t pin, uint32_t value)
{
    gpio_set_output_value(pin, value ? 1 : 0);
    return H_OK;
}

static int h_gpio_pull_adapter(uint32_t pin, uint32_t pull_type, bool enable)
{
    if (pull_type == H_GPIO_PULL_UP) {
        gpio_set_pull_up(pin, enable ? 1 : 0);
    } else {
        gpio_set_pull_down(pin, enable ? 1 : 0);
    }
    return H_OK;
}

static int h_gpio_hold_adapter(uint32_t pin, bool enable)
{
    (void)pin;
    (void)enable;
    return H_ERR_NOT_SUP;
}

extern int esp_hosted_tx(uint8_t iface_type, uint8_t iface_num,
                         uint8_t *payload, uint16_t len, uint8_t zcopy,
                         uint8_t *to_free, void (*free_fn)(void *), uint8_t flags);

static int h_esp_hosted_tx_adapter(uint8_t iface_type, uint8_t iface_num,
                                   uint8_t *payload, uint16_t len,
                                   uint8_t zcopy, void *to_free,
                                   void (*free_fn)(void *), uint8_t flags)
{
    return esp_hosted_tx(iface_type, iface_num, payload, len, zcopy,
                         (uint8_t *)to_free, free_fn, flags);
}

void *jl_uart_get_handle(void)
{
    return g_uart_handle;
}

static void jl_post_transport_init(void)
{
    extern void jl_uart_transport_start(void);
    jl_uart_transport_start();
}

const h_transport_contract_t g_h_transport = {
    .init           = h_uart_init_adapter,
    .deinit         = h_uart_deinit_adapter,
    .bus_ready      = ensure_slave_bus_ready,
    .transmit       = h_esp_hosted_tx_adapter,

    .spi_transfer   = NULL,

    .sdio_card_init = NULL,
    .sdio_read_reg  = NULL,
    .sdio_write_reg = NULL,
    .sdio_read_block = NULL,
    .sdio_write_block = NULL,
    .sdio_wait_intr = NULL,

    .uart_read      = h_uart_read_adapter,
    .uart_write     = h_uart_write_adapter,
    .uart_flush     = h_uart_flush_adapter,

    .gpio_config    = h_gpio_config_adapter,
    .gpio_set_intr  = h_gpio_set_intr_adapter,
    .gpio_clear_intr = h_gpio_clear_intr_adapter,
    .gpio_read      = h_gpio_read_adapter,
    .gpio_write     = h_gpio_write_adapter,
    .gpio_pull      = h_gpio_pull_adapter,
    .gpio_hold      = h_gpio_hold_adapter,

    .netif_create   = NULL,
    .netif_destroy  = NULL,

    .post_transport_init_hook = jl_post_transport_init,
};

#endif /* H_TRANSPORT_IN_USE == H_TRANSPORT_UART */
