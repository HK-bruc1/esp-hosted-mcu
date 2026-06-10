/* ESP-IDF UART Transport port — bus adapter + contract assembly. */

#include "h_port_contract.h"
#include "h_port_config.h"

#if H_TRANSPORT_IN_USE == H_TRANSPORT_UART

#include <esp_err.h>
#include "driver/gpio.h"
#include "h_transport_gpio.h"
#include "h_transport_uart_bus.h"
#include "hci_drv.h"
/* Transition: reuse shared types/macros from legacy transport_drv.h.
 * Can be removed when ESP-IDF port is fully self-contained. */
#include "transport_drv.h"

/* ──  Adapters ── */

static int h_uart_init_adapter(void **out_handle)
{
    *out_handle = h_uart_bus_init();
    return (*out_handle != NULL) ? H_OK : H_FAIL;
}

static int h_uart_deinit_adapter(void *handle)
{
    esp_err_t ret = h_uart_bus_deinit(handle);
    return (ret == ESP_OK) ? H_OK : H_FAIL;
}

/* h_uart_bus_read returns the number of bytes read (int); negative on error.
 * The contract also returns int — pass-through with error translation. */
static int h_uart_read_adapter(void *handle, uint8_t *data, uint16_t size)
{
    int ret = h_uart_bus_read(handle, data, size);
    return (ret >= 0) ? ret : H_FAIL;
}

/* h_uart_bus_write returns the number of bytes written (int); negative on error */
static int h_uart_write_adapter(void *handle, uint8_t *data, uint16_t size)
{
    int ret = h_uart_bus_write(handle, data, size);
    return (ret >= 0) ? ret : H_FAIL;
}

static int h_uart_flush_adapter(void *handle)
{
    int ret = h_uart_bus_flush_input(handle);
    return (ret == ESP_OK) ? H_OK : H_FAIL;
}

/* GPIO */
static int h_gpio_config_adapter(uint32_t pin, uint32_t mode)
{
    espidf_gpio_config(pin, mode);
    return H_OK;
}

static int h_gpio_set_intr_adapter(uint32_t pin, uint32_t intr_type,
                                   void (*isr)(void*), void *arg)
{
    espidf_gpio_setup_intr(pin, intr_type, isr, arg);
    return H_OK;
}

static int h_gpio_clear_intr_adapter(uint32_t pin)
{
    espidf_gpio_clear_intr(pin);
    return H_OK;
}

static int h_gpio_read_adapter(uint32_t pin)
{
    return espidf_gpio_read(pin);
}

static int h_gpio_write_adapter(uint32_t pin, uint32_t value)
{
    espidf_gpio_write(pin, value);
    return H_OK;
}

static int h_gpio_pull_adapter(uint32_t pin, uint32_t pull_type, bool enable)
{
    if (pull_type == GPIO_PULLUP_ONLY) {
        return enable ? gpio_pullup_en(pin) : gpio_pullup_dis(pin);
    } else {
        return enable ? gpio_pulldown_en(pin) : gpio_pulldown_dis(pin);
    }
}

static int h_gpio_hold_adapter(uint32_t pin, bool enable)
{
    return enable ? gpio_hold_en(pin) : gpio_hold_dis(pin);
}

static int h_esp_hosted_tx_adapter(uint8_t iface_type, uint8_t iface_num,
                                   uint8_t *payload, uint16_t len,
                                   uint8_t zcopy, void *to_free,
                                   void (*free_fn)(void *), uint8_t flags)
{
    return esp_hosted_tx(iface_type, iface_num, payload, len, zcopy,
                         (uint8_t *)to_free, free_fn, flags);
}

/* ──  Global Transport Contract Instance (UART) ── */

const h_transport_contract_t g_h_transport = {
    .init           = h_uart_init_adapter,
    .deinit         = h_uart_deinit_adapter,
    .bus_ready      = ensure_slave_bus_ready,
    .transmit       = h_esp_hosted_tx_adapter,

    /* SPI — not used in UART transport */
    .spi_transfer   = NULL,

    /* SDIO — not used in UART transport */
    .sdio_card_init = NULL,
    .sdio_read_reg  = NULL,
    .sdio_write_reg = NULL,
    .sdio_read_block = NULL,
    .sdio_write_block = NULL,
    .sdio_wait_intr = NULL,

    /* UART */
    .uart_read      = h_uart_read_adapter,
    .uart_write     = h_uart_write_adapter,
    .uart_flush     = h_uart_flush_adapter,

    /* GPIO */
    .gpio_config    = h_gpio_config_adapter,
    .gpio_set_intr  = h_gpio_set_intr_adapter,
    .gpio_clear_intr = h_gpio_clear_intr_adapter,
    .gpio_read      = h_gpio_read_adapter,
    .gpio_write     = h_gpio_write_adapter,
    .gpio_pull      = h_gpio_pull_adapter,
    .gpio_hold      = h_gpio_hold_adapter,

    /* netif — NULL for Phase 1 */
    .netif_create   = NULL,
    .netif_destroy  = NULL,

    .post_transport_init_hook = hci_drv_init,
};

#endif /* H_TRANSPORT_IN_USE == H_TRANSPORT_UART */
