/* host/port/jl/h_transport_task.c
 * JL UART transport task layer.
 */

#include "h_port_config.h"
#include "h_wrapper.h"
#include "h_transport_drv.h"
#include "transport_drv_api.h"
#include "h_transport_uart_bus.h"
#include "esp_hosted_transport.h"
#include "esp_hosted_header.h"
#include <string.h>

#define TAG "H_UART_TASK"

static h_queue_t g_tx_queue = NULL;
static h_thread_t g_tx_thread = NULL;
static h_thread_t g_rx_thread = NULL;
static volatile bool g_tx_running = false;
static volatile bool g_rx_running = false;
static volatile bool g_tx_exited = false;
static volatile bool g_rx_exited = false;

#define TX_QUEUE_SIZE  H_UART_TX_QUEUE_SIZE

/* --------------------------------------------------------------------------
 * Frame helpers
 * -------------------------------------------------------------------------- */

static inline uint16_t htole16(uint16_t v)
{
    return v;
}

static inline uint16_t le16toh(uint16_t v)
{
    return v;
}

static uint8_t *jl_alloc_tx_buffer(uint32_t need_memset)
{
    uint8_t *buf = (uint8_t *)malloc(MAX_UART_BUFFER_SIZE);
    if (buf && need_memset) {
        memset(buf, 0, MAX_UART_BUFFER_SIZE);
    }
    return buf;
}

static void jl_free_tx_buffer(void *buf)
{
    free(buf);
}

/* --------------------------------------------------------------------------
 * TX path
 * -------------------------------------------------------------------------- */

static int jl_uart_write_packet(interface_buffer_handle_t *buf_handle)
{
    if (!buf_handle) {
        return H_FAIL;
    }

    uint16_t len = buf_handle->payload_len;
    if (!buf_handle->flag && !len) {
        H_LOGE(TAG, "Empty len");
        return H_FAIL;
    }

    uint8_t *sendbuf = NULL;
    void (*free_func)(void *) = NULL;
    if (!buf_handle->payload_zcopy) {
        sendbuf = jl_alloc_tx_buffer(1);
        if (!sendbuf) {
            return H_FAIL;
        }
        free_func = jl_free_tx_buffer;
    } else {
        sendbuf = buf_handle->payload;
        free_func = buf_handle->free_buf_handle;
    }

    if (buf_handle->payload_len > MAX_PAYLOAD_SIZE) {
        H_LOGE(TAG, "Pkt len too big");
        if (free_func) {
            free_func(sendbuf);
        }
        return H_FAIL;
    }

    struct esp_payload_header *hdr = (struct esp_payload_header *)sendbuf;
    uint8_t *payload = sendbuf + sizeof(struct esp_payload_header);

    memset(hdr, 0, sizeof(*hdr));
    hdr->len = htole16(len);
    hdr->offset = htole16(sizeof(struct esp_payload_header));
    hdr->if_type = buf_handle->if_type;
    hdr->if_num = buf_handle->if_num;
    hdr->seq_num = htole16(buf_handle->seq_num);
    hdr->flags = buf_handle->flag;

    if (hdr->if_type == ESP_HCI_IF && !buf_handle->payload_zcopy) {
        hdr->hci_pkt_type = buf_handle->payload[0];
        len -= 1;
        hdr->len = htole16(len);
        memcpy(payload, &buf_handle->payload[1], len);
    } else if (!buf_handle->payload_zcopy) {
        memcpy(payload, buf_handle->payload, len);
    }

#if H_UART_CHECKSUM
    hdr->checksum = htole16(compute_checksum(sendbuf,
        sizeof(struct esp_payload_header) + len));
#endif

    void *uart = jl_uart_get_handle();
    int tx_len_to_send = len + sizeof(struct esp_payload_header);
    int tx_len = jl_uart_bus_write(uart, sendbuf, tx_len_to_send);

    int result = H_OK;
    if (tx_len != tx_len_to_send) {
        H_LOGE(TAG, "UART write failed");
        result = H_FAIL;
    }

    if (free_func && !buf_handle->payload_zcopy) {
        free_func(sendbuf);
    }
    return result;
}

static void jl_uart_tx_task(void *arg)
{
    (void)arg;
    g_tx_running = true;

    interface_buffer_handle_t buf_handle;
    while (g_tx_running) {
        if (h_queue_recv(g_tx_queue, &buf_handle, H_BLOCK_MAX) != H_OK) {
            continue;
        }

        /* Sentinel from jl_uart_transport_stop(): exit without transmitting. */
        if (buf_handle.payload_zcopy == 0xFF) {
            g_tx_running = false;
            break;
        }

        jl_uart_write_packet(&buf_handle);
        if (buf_handle.free_buf_handle && buf_handle.priv_buffer_handle) {
            buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
        }
    }
    g_tx_exited = true;
    g_tx_running = false;
}

int esp_hosted_tx(uint8_t iface_type, uint8_t iface_num,
                  uint8_t *payload_buf, uint16_t payload_len,
                  uint8_t buff_zerocopy, uint8_t *buffer_to_free,
                  void (*free_buf_func)(void *ptr), uint8_t flags)
{
    if (!g_tx_queue) {
        H_LOGE(TAG, "TX queue not ready");
        return H_FAIL;
    }

    interface_buffer_handle_t buf_handle = {0};
    buf_handle.if_type = iface_type;
    buf_handle.if_num = iface_num;
    buf_handle.payload = payload_buf;
    buf_handle.payload_len = payload_len;
    buf_handle.payload_zcopy = buff_zerocopy;
    buf_handle.priv_buffer_handle = buffer_to_free;
    buf_handle.free_buf_handle = free_buf_func;
    buf_handle.flag = flags;

    int ret = h_queue_send(g_tx_queue, &buf_handle, H_BLOCK_MAX);
    if (ret != H_OK) {
        H_LOGE(TAG, "TX queue full");
        return H_FAIL;
    }
    return H_OK;
}

/* --------------------------------------------------------------------------
 * RX path
 * -------------------------------------------------------------------------- */

extern int serial_rx_handler(interface_buffer_handle_t *buf_handle);
extern void process_priv_communication(interface_buffer_handle_t *buf_handle);
extern transport_channel_t *chan_arr[ESP_MAX_IF];

static int jl_uart_read_fully(uint8_t *dst, uint16_t len)
{
    uint16_t got = 0;
    void *uart = jl_uart_get_handle();
    while (got < len) {
        if (!g_rx_running) {
            return H_FAIL;
        }
        int ret = jl_uart_bus_read(uart, dst + got, len - got);
        if (ret < 0) {
            return H_FAIL;
        }
        if (ret == 0) {
            h_msleep(1);
            continue;
        }
        got += (uint16_t)ret;
    }
    return H_OK;
}

static bool jl_uart_validate_header(struct esp_payload_header *h,
                                    uint16_t *out_len)
{
    if (!h || !out_len) {
        return false;
    }

    uint16_t len = le16toh(h->len);
    uint16_t offset = le16toh(h->offset);

    if ((!len) || (len > MAX_PAYLOAD_SIZE) ||
        (offset != sizeof(struct esp_payload_header))) {
        H_LOGE(TAG, "Invalid RX header: len=%u offset=%u", len, offset);
        return false;
    }

    if (h->if_type >= ESP_MAX_IF) {
        H_LOGE(TAG, "Invalid RX if_type: %u", h->if_type);
        return false;
    }

    *out_len = len;
    return true;
}

static bool jl_uart_read_packet(struct esp_payload_header *out_hdr,
                                uint8_t **out_payload,
                                uint16_t *out_payload_len)
{
    if (!out_hdr || !out_payload || !out_payload_len) {
        return false;
    }

    struct esp_payload_header hdr;

    while (g_rx_running) {
        if (jl_uart_read_fully((uint8_t *)&hdr, sizeof(hdr)) != H_OK) {
            if (!g_rx_running) {
                return false;
            }
            continue;
        }

        uint16_t payload_len = 0;
        if (!jl_uart_validate_header(&hdr, &payload_len)) {
            jl_uart_bus_flush_input(jl_uart_get_handle());
            continue;
        }

        /* Allocate a contiguous buffer for header + payload so checksum
         * can be verified against the actual received bytes. */
        uint16_t total_len = sizeof(hdr) + payload_len;
        if (total_len < sizeof(hdr)) {
            H_LOGE(TAG, "RX total_len overflow");
            jl_uart_bus_flush_input(jl_uart_get_handle());
            continue;
        }

        uint8_t *rx_packet = (uint8_t *)malloc(total_len);
        if (!rx_packet) {
            H_LOGE(TAG, "RX malloc failed");
            jl_uart_bus_flush_input(jl_uart_get_handle());
            continue;
        }
        memcpy(rx_packet, &hdr, sizeof(hdr));

        if (payload_len) {
            if (jl_uart_read_fully(rx_packet + sizeof(hdr), payload_len) != H_OK) {
                free(rx_packet);
                continue;
            }
        }

#if H_UART_CHECKSUM
        struct esp_payload_header *chk_hdr = (struct esp_payload_header *)rx_packet;
        uint16_t rx_checksum = le16toh(chk_hdr->checksum);
        chk_hdr->checksum = 0;
        uint16_t checksum = compute_checksum(rx_packet, total_len);
        if (checksum != rx_checksum) {
            H_LOGE(TAG, "RX checksum mismatch: rx=%u calc=%u", rx_checksum, checksum);
            free(rx_packet);
            jl_uart_bus_flush_input(jl_uart_get_handle());
            continue;
        }
#endif

        *out_payload = rx_packet + sizeof(struct esp_payload_header);
        *out_payload_len = payload_len;
        *out_hdr = hdr;
        return true;
    }

    return false;
}

static void jl_uart_rx_task(void *arg)
{
    (void)arg;
    g_rx_running = true;

    while (g_rx_running) {
        if (is_transport_rx_ready()) {
            break;
        }
        h_msleep(100);
    }

    while (g_rx_running) {
        struct esp_payload_header hdr;
        uint8_t *payload = NULL;
        uint16_t payload_len = 0;

        if (!jl_uart_read_packet(&hdr, &payload, &payload_len)) {
            continue;
        }

        interface_buffer_handle_t buf_handle = {0};
        buf_handle.if_type = hdr.if_type;
        buf_handle.if_num = hdr.if_num;
        buf_handle.payload = payload;
        buf_handle.payload_len = payload_len;
        buf_handle.flag = hdr.flags;
        buf_handle.seq_num = hdr.seq_num;
        buf_handle.payload_zcopy = H_BUFF_NO_ZEROCOPY;
        buf_handle.priv_buffer_handle = payload - sizeof(struct esp_payload_header);
        buf_handle.free_buf_handle = (void (*)(void *))free;

        if (buf_handle.if_type == ESP_SERIAL_IF) {
            serial_rx_handler(&buf_handle);
        } else if (buf_handle.if_type == ESP_PRIV_IF) {
            process_priv_communication(&buf_handle);
        } else if ((buf_handle.if_type == ESP_STA_IF ||
                    buf_handle.if_type == ESP_AP_IF) &&
                   chan_arr[buf_handle.if_type] &&
                   chan_arr[buf_handle.if_type]->rx) {
            uint8_t *copy = (uint8_t *)h_malloc(buf_handle.payload_len);
            if (copy) {
                memcpy(copy, buf_handle.payload, buf_handle.payload_len);
                int ret = chan_arr[buf_handle.if_type]->rx(
                    chan_arr[buf_handle.if_type]->api_chan,
                    copy, copy, buf_handle.payload_len);
                if (ret) {
                    h_free(copy);
                }
            } else {
                H_LOGE(TAG, "No mem for STA/AP copy");
            }
        } else if (buf_handle.if_type == ESP_HCI_IF) {
            H_LOGW(TAG, "BT HCI RX dropped (Phase 1)");
        } else {
            H_LOGW(TAG, "unknown if_type %d", buf_handle.if_type);
        }

        if (buf_handle.free_buf_handle && buf_handle.priv_buffer_handle) {
            buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
        }
    }
    g_rx_exited = true;
    g_rx_running = false;
}

/* --------------------------------------------------------------------------
 * Public init / deinit
 * -------------------------------------------------------------------------- */

int ensure_slave_bus_ready(void *bus_handle)
{
    (void)bus_handle;

    H_LOGI(TAG, "Resetting slave on UART bus with pin %d", H_GPIO_PIN_RESET);
    h_gpio_config(H_GPIO_PIN_RESET, H_GPIO_MODE_OUTPUT);
    h_gpio_write(H_GPIO_PIN_RESET, H_RESET_VAL_ACTIVE);
    h_msleep(10);
    h_gpio_write(H_GPIO_PIN_RESET, H_RESET_VAL_INACTIVE);
    h_msleep(10);
    h_gpio_write(H_GPIO_PIN_RESET, H_RESET_VAL_ACTIVE);
    jl_uart_bus_flush_input(jl_uart_get_handle());
    h_msleep(1500);
    return H_OK;
}

void jl_uart_transport_start(void)
{
    if (g_tx_queue) {
        return;
    }

    g_tx_exited = false;
    g_rx_exited = false;

    if (h_queue_create(TX_QUEUE_SIZE, sizeof(interface_buffer_handle_t),
                        &g_tx_queue) != H_OK) {
        return;
    }

    if (h_thread_create("h_uart_tx", H_DEFAULT_TASK_PRIO, H_DEFAULT_TASK_STACK,
                        jl_uart_tx_task, NULL, &g_tx_thread) != H_OK ||
        h_thread_create("h_uart_rx", H_DEFAULT_TASK_PRIO, H_DEFAULT_TASK_STACK,
                        jl_uart_rx_task, NULL, &g_rx_thread) != H_OK) {
        H_LOGE(TAG, "Failed to create UART tasks");
        jl_uart_transport_stop();
        return;
    }
}

void jl_uart_transport_stop(void)
{
    /* Signal threads to exit. */
    g_tx_running = false;
    g_rx_running = false;

    /* Drain any pending TX work and wake the TX task so it observes stop. */
    if (g_tx_queue) {
        interface_buffer_handle_t sentinel = {0};
        sentinel.payload_zcopy = 0xFF; /* marker: stop sentinel */
        h_queue_send(g_tx_queue, &sentinel, 0);
    }

    /* Wait for tasks to signal completion. Use a bounded poll because JL
     * os_task_del requires the task name and we cannot join on a handle. */
    for (int i = 0; i < 100 && !g_tx_exited; i++) {
        h_msleep(10);
    }
    for (int i = 0; i < 100 && !g_rx_exited; i++) {
        h_msleep(10);
    }

    if (g_tx_thread) {
        h_thread_delete(g_tx_thread);
        g_tx_thread = NULL;
    }
    if (g_rx_thread) {
        h_thread_delete(g_rx_thread);
        g_rx_thread = NULL;
    }

    if (g_tx_queue) {
        h_queue_delete(g_tx_queue);
        g_tx_queue = NULL;
    }
}
