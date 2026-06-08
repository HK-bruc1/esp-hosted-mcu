/* ESP-IDF SPI-HD Transport port — bus adapter + contract assembly. */

#include "h_port_contract.h"
#include "h_port_config.h"

#if H_TRANSPORT_IN_USE == H_TRANSPORT_SPI_HD

#include <esp_err.h>
#include <driver/gpio.h>
#include "h_transport_gpio.h"

/* ── Bus helpers (from h_transport_spi_hd_bus.c) ── */
#include "h_transport_spi_hd_bus.h"
/* ── Driver API (from transport_drv_api.h) ── */
#include "transport_drv_api.h"


/* ── Adapters ── */

static int h_spi_hd_init_adapter(void **out_handle)
{
    *out_handle = h_spi_hd_bus_init();
    return (*out_handle != NULL) ? H_OK : H_FAIL;
}

static int h_spi_hd_deinit_adapter(void *handle)
{
    int ret = h_spi_hd_bus_deinit(handle);
    return (ret == 0) ? H_OK : H_FAIL;
}

/* SPI-HD specific operations — hosted_* functions currently use global ctx,
 * so the handle parameter is passed but not used internally.
 * This is a known limitation that will be addressed when multi-device
 * support is needed. */
static int h_spi_hd_read_reg_adapter(void *handle, uint32_t reg,
                                      uint32_t *data, int poll, bool lock)
{
    (void)handle;
    return h_spi_hd_bus_read_reg(reg, data, poll, lock);
}

static int h_spi_hd_write_reg_adapter(void *handle, uint32_t reg,
                                       uint32_t *data, bool lock)
{
    (void)handle;
    return h_spi_hd_bus_write_reg(reg, data, lock);
}

static int h_spi_hd_read_dma_adapter(void *handle, uint8_t *data,
                                      uint16_t size, bool lock)
{
    (void)handle;
    return h_spi_hd_bus_read_dma(data, size, lock);
}

static int h_spi_hd_write_dma_adapter(void *handle, uint8_t *data,
                                       uint16_t size, bool lock)
{
    (void)handle;
    return h_spi_hd_bus_write_dma(data, size, lock);
}

static int h_spi_hd_send_cmd9_adapter(void *handle)
{
    (void)handle;
    return h_spi_hd_bus_send_cmd9();
}

/* GPIO — old functions take (gpio_port, pin, ...), contract takes (pin, ...) */
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

/* ── Global Transport Contract Instance (SPI-HD) ── */

const h_transport_contract_t g_h_transport = {
    .init           = h_spi_hd_init_adapter,
    .deinit         = h_spi_hd_deinit_adapter,
    .bus_ready      = ensure_slave_bus_ready,
    .transmit       = h_esp_hosted_tx_adapter,

    /* SPI Full-Duplex — not used in SPI-HD transport */
    .spi_transfer   = NULL,

    /* SPI-HD */
    .spi_hd_read_reg  = h_spi_hd_read_reg_adapter,
    .spi_hd_write_reg = h_spi_hd_write_reg_adapter,
    .spi_hd_read_dma  = h_spi_hd_read_dma_adapter,
    .spi_hd_write_dma = h_spi_hd_write_dma_adapter,
    .spi_hd_send_cmd9 = h_spi_hd_send_cmd9_adapter,

    /* SDIO — not used in SPI-HD transport */
    .sdio_card_init   = NULL,
    .sdio_read_reg    = NULL,
    .sdio_write_reg   = NULL,
    .sdio_read_block  = NULL,
    .sdio_write_block = NULL,
    .sdio_wait_intr   = NULL,

    /* UART — not used in SPI-HD transport */
    .uart_read        = NULL,
    .uart_write       = NULL,
    .uart_flush       = NULL,

    /* GPIO */
    .gpio_config      = h_gpio_config_adapter,
    .gpio_set_intr    = h_gpio_set_intr_adapter,
    .gpio_clear_intr  = h_gpio_clear_intr_adapter,
    .gpio_read        = h_gpio_read_adapter,
    .gpio_write       = h_gpio_write_adapter,
    .gpio_pull        = h_gpio_pull_adapter,
    .gpio_hold        = h_gpio_hold_adapter,

    /* netif — NULL for Phase 1 (ESP-IDF uses static netif creation) */
    .netif_create     = NULL,
    .netif_destroy    = NULL,
};

#endif /* H_TRANSPORT_IN_USE == H_TRANSPORT_SPI_HD */
