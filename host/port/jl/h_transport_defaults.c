/* host/port/jl/h_transport_defaults.c
 * JL default transport configurations.
 *
 * Phase 1 intentionally bypasses the ESP-IDF transport-config layer: the
 * UART bus is initialized directly by h_port_transport_init() using values
 * from h_port_config.h.  The two generic config helpers below exist only
 * because esp_hosted_api.c links against them.  If dynamic transport
 * configuration is introduced later, these stubs must be replaced with real
 * implementations.
 */

#include "h_port_config.h"
#include "esp_hosted_transport_config.h"
#include "esp_err.h"

/* Phase 1: no dynamic transport configuration exists.  Always report valid
 * so that esp_hosted_init() can proceed with the statically selected UART
 * transport.  Replace with real validation when CONFIG_ESP_HOSTED_* bus
 * selection becomes runtime-configurable. */
bool esp_hosted_is_config_valid(void)
{
    return true;
}

/* Phase 1: the UART bus is already configured by h_port_config.h and
 * initialized in h_port_transport_init().  Nothing to set here. */
esp_err_t esp_hosted_set_default_config(void)
{
    return ESP_OK;
}

#if CONFIG_ESP_HOSTED_UART_HOST_INTERFACE
struct esp_hosted_uart_config esp_hosted_get_default_uart_config(void)
{
    return (struct esp_hosted_uart_config) {
        .port = H_UART_PORT,
        .pin_tx = {.port = H_UART_PORT_TX, .pin = H_UART_PIN_TX},
        .pin_rx = {.port = H_UART_PORT_RX, .pin = H_UART_PIN_RX},
        .pin_reset = {.port = H_GPIO_PORT_RESET, .pin = H_GPIO_PIN_RESET},
        .num_data_bits = H_UART_NUM_DATA_BITS,
        .parity = H_UART_PARITY,
        .stop_bits = H_UART_STOP_BITS,
        .flow_ctrl = H_UART_FLOWCTRL,
        .clk_src = H_UART_CLK_SRC,
        .checksum_enable = H_UART_CHECKSUM,
        .baud_rate = H_UART_BAUD_RATE,
        .tx_queue_size = H_UART_TX_QUEUE_SIZE,
        .rx_queue_size = H_UART_RX_QUEUE_SIZE,
    };
}
#endif
