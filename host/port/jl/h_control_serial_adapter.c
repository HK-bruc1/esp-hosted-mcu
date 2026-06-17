/* host/port/jl/h_control_serial_adapter.c
 * JL control serial adapter.
 */

#include <string.h>
#include "serial_ll_if.h"
#include "serial_if.h"
#include "h_wrapper.h"
#include "h_control_serial_contract.h"

DEFINE_LOG_TAG(serial);

struct h_control_serial_handle {
    int handle;
};

static serial_ll_handle_t *serial_ll_if_g = NULL;
static h_semaphore_t read_semaphore = NULL;
static struct h_control_serial_handle *g_serial_drv_handle = NULL;

static void rpc_rx_indication(void)
{
    if (read_semaphore) {
        h_sem_give(read_semaphore);
    }
}

static int h_control_serial_drv_write_fail(uint8_t *buf, int *out_count, int ret)
{
    if (out_count) {
        *out_count = 0;
    }
    if (buf) {
        h_free(buf);
    }
    return ret;
}

h_control_serial_handle_t *h_control_serial_drv_open(const char *transport)
{
    (void)transport;

    if (g_serial_drv_handle) {
        return g_serial_drv_handle;
    }

    g_serial_drv_handle = (struct h_control_serial_handle *)
        h_calloc(1, sizeof(struct h_control_serial_handle));
    if (!g_serial_drv_handle) {
        H_LOGE(TAG, "Failed to allocate memory");
        return NULL;
    }
    return g_serial_drv_handle;
}

int h_control_serial_drv_close(h_control_serial_handle_t **handle)
{
    if (!handle || !(*handle)) {
        return H_ERR_INVALID_ARG;
    }
    h_free(*handle);
    *handle = NULL;
    g_serial_drv_handle = NULL;
    return H_OK;
}

int h_control_serial_drv_write(h_control_serial_handle_t *handle,
                               uint8_t *buf, int in_count, int *out_count)
{
    if (!handle || !buf || !in_count || !out_count) {
        if (out_count) {
            *out_count = 0;
        }
        H_LOGE(TAG, "Invalid parameters in write");
        return H_ERR_INVALID_ARG;
    }

    if (!serial_ll_if_g || !serial_ll_if_g->fops ||
        !serial_ll_if_g->fops->write) {
        H_LOGE(TAG, "serial interface not valid");
        return h_control_serial_drv_write_fail(buf, out_count, H_ERR_INVALID_ARG);
    }

    int ret = serial_ll_if_g->fops->write(serial_ll_if_g, buf, in_count);
    if (ret != H_OK) {
        *out_count = 0;
        H_LOGE(TAG, "Failed to write data");
        return H_FAIL;
    }

    *out_count = in_count;
    return H_OK;
}

uint8_t *h_control_serial_drv_read(h_control_serial_handle_t *handle,
                                   uint32_t *out_nbyte)
{
    if (!handle || !out_nbyte) {
        H_LOGE(TAG, "Invalid parameters in read");
        return NULL;
    }

    *out_nbyte = 0;

    if (!read_semaphore) {
        H_LOGE(TAG, "Semaphore not initialized");
        return NULL;
    }

    h_sem_take(read_semaphore, H_BLOCK_MAX);

    if (!serial_ll_if_g || !serial_ll_if_g->fops ||
        !serial_ll_if_g->fops->read) {
        H_LOGE(TAG, "serial interface refusing to read");
        return NULL;
    }

    uint16_t rx_buf_len = 0;
    uint8_t *read_buf = serial_ll_if_g->fops->read(serial_ll_if_g, &rx_buf_len);
    if (!read_buf || !rx_buf_len) {
        H_LOGE(TAG, "serial read failed");
        return NULL;
    }

    const char *ep_name = RPC_EP_NAME_RSP;
    uint16_t init_read_len = SIZE_OF_TYPE + SIZE_OF_LENGTH + strlen(ep_name) +
                             SIZE_OF_TYPE + SIZE_OF_LENGTH;

    if (rx_buf_len < init_read_len) {
        h_free(read_buf);
        H_LOGE(TAG, "Incomplete serial buff");
        return NULL;
    }

    uint8_t *buf = h_calloc(1, init_read_len);
    if (!buf) {
        h_free(read_buf);
        return NULL;
    }

    h_memcpy(buf, read_buf, init_read_len);

    uint32_t payload_len = 0;
    int ret = parse_tlv(buf, &payload_len);
    if (ret || !payload_len) {
        h_free(buf);
        h_free(read_buf);
        H_LOGE(TAG, "Failed to parse RX data");
        return NULL;
    }

    if (rx_buf_len < (init_read_len + payload_len)) {
        h_free(buf);
        h_free(read_buf);
        H_LOGE(TAG, "Buf read smaller than expected");
        return NULL;
    }

    h_free(buf);

    buf = h_calloc(1, payload_len);
    if (!buf) {
        h_free(read_buf);
        return NULL;
    }

    h_memcpy(buf, read_buf + init_read_len, payload_len);
    h_free(read_buf);

    *out_nbyte = payload_len;
    return buf;
}

int h_control_serial_platform_init(void)
{
    if (h_sem_create(H_MAX_SYNC_RPC_REQUESTS + H_MAX_ASYNC_RPC_REQUESTS,
                     0, &read_semaphore) != H_OK) {
        H_LOGE(TAG, "Failed to create RPC semaphore");
        return H_FAIL;
    }

    serial_ll_if_g = serial_ll_init(rpc_rx_indication);
    if (!serial_ll_if_g) {
        H_LOGE(TAG, "Serial interface creation failed");
        h_sem_delete(read_semaphore);
        read_semaphore = NULL;
        return H_FAIL;
    }

    if (serial_ll_if_g->fops->open(serial_ll_if_g) != H_OK) {
        H_LOGE(TAG, "Serial interface open failed");
        serial_ll_if_g->fops->close(serial_ll_if_g);
        serial_ll_if_g = NULL;
        h_sem_delete(read_semaphore);
        read_semaphore = NULL;
        return H_FAIL;
    }

    return H_OK;
}

int h_control_serial_platform_deinit(void)
{
    if (serial_ll_if_g) {
        serial_ll_if_g->fops->close(serial_ll_if_g);
        serial_ll_if_g = NULL;
    }

    if (read_semaphore) {
        h_sem_delete(read_semaphore);
        read_semaphore = NULL;
    }

    return H_OK;
}
