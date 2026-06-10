/* ESP-IDF SDIO Transport port — bus adapter + contract assembly. */

#include "h_port_contract.h"
#include "h_port_config.h"
#include "h_wrapper.h"

#if H_TRANSPORT_IN_USE == H_TRANSPORT_SDIO

#include <esp_err.h>
#include "driver/gpio.h"
#include "h_transport_gpio.h"
#include "freertos/FreeRTOS.h"  /* pdMS_TO_TICKS */

/* ──  SDIO bus helpers (from host/port/esp-idf/) ── */
#include "h_transport_sdio_bus.h"
#include "hci_drv.h"
#include "transport_drv_api.h"


/* ──  Adapters ── */

static int h_sdio_init_adapter(void **out_handle)
{
    *out_handle = h_sdio_bus_init();
    return (*out_handle != NULL) ? H_OK : H_FAIL;
}

static int h_sdio_deinit_adapter(void *handle)
{
    /* Card deinit releases DMA-aligned buffers allocated by sdmmc_card_init.
     * Fold into adapter deinit so that driver-level h_transport_deinit()
     * handles the full SDIO teardown in one call. */
    h_sdio_bus_card_deinit(handle);
    int ret = h_sdio_bus_deinit(handle);
    return (ret == 0) ? H_OK : H_FAIL;
}

/* SDIO card init — pass-through */
static int h_sdio_card_init_adapter(void *handle, bool show_config)
{
    int ret = h_sdio_bus_card_init(handle, show_config);
    return (ret == 0) ? H_OK : H_FAIL;
}

static int h_sdio_read_reg_adapter(void *handle, uint32_t reg, uint8_t *data,
                                   uint16_t size, bool lock)
{
    int ret = h_sdio_bus_read_reg(handle, reg, data, size, lock);
    return (ret == 0) ? H_OK : H_FAIL;
}

static int h_sdio_write_reg_adapter(void *handle, uint32_t reg, uint8_t *data,
                                    uint16_t size, bool lock)
{
    int ret = h_sdio_bus_write_reg(handle, reg, data, size, lock);
    return (ret == 0) ? H_OK : H_FAIL;
}

static int h_sdio_read_block_adapter(void *handle, uint32_t reg,
                                     uint8_t *data, uint16_t size, bool lock)
{
    int ret = h_sdio_bus_read_block(handle, reg, data, size, lock);
    return (ret == 0) ? H_OK : H_FAIL;
}

static int h_sdio_write_block_adapter(void *handle, uint32_t reg,
                                      uint8_t *data, uint16_t size, bool lock)
{
    int ret = h_sdio_bus_write_block(handle, reg, data, size, lock);
    return (ret == 0) ? H_OK : H_FAIL;
}

/* h_sdio_bus_wait_intr takes FreeRTOS ticks; contract says timeout_ms.
 * H_BLOCK_FOREVER (-1) is defined in h_wrapper.h as the sentinel for
 * "block forever". When cast to uint32_t it becomes UINT32_MAX; we detect
 * this and substitute portMAX_DELAY to avoid pdMS_TO_TICKS overflow. */
static int h_sdio_wait_intr_adapter(void *handle, uint32_t timeout_ms)
{
    uint32_t ticks = (timeout_ms == (uint32_t)H_BLOCK_FOREVER)
                     ? portMAX_DELAY
                     : pdMS_TO_TICKS(timeout_ms);
    int ret = h_sdio_bus_wait_intr(handle, ticks);
    return (ret == 0) ? H_OK : H_ERR_TIMEOUT;
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

/* ──  Global Transport Contract Instance (SDIO) ── */

const h_transport_contract_t g_h_transport = {
    .init           = h_sdio_init_adapter,
    .deinit         = h_sdio_deinit_adapter,
    .bus_ready      = ensure_slave_bus_ready,
    .transmit       = h_esp_hosted_tx_adapter,

    /* SPI — not used in SDIO transport */
    .spi_transfer   = NULL,

    /* SDIO */
    .sdio_card_init = h_sdio_card_init_adapter,
    .sdio_read_reg  = h_sdio_read_reg_adapter,
    .sdio_write_reg = h_sdio_write_reg_adapter,
    .sdio_read_block = h_sdio_read_block_adapter,
    .sdio_write_block = h_sdio_write_block_adapter,
    .sdio_wait_intr = h_sdio_wait_intr_adapter,

    /* UART — not used in SDIO transport */
    .uart_read      = NULL,
    .uart_write     = NULL,
    .uart_flush     = NULL,

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

#endif /* H_TRANSPORT_IN_USE == H_TRANSPORT_SDIO */
