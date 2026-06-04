/* host/core/src/h_serial_if.c
 *
 * Serial interface for control path communication.
 * Routes protobuf-encoded RPC messages over the transport layer. */

#include "h_serial_if.h"
#include "h_wrapper.h"
#include "esp_hosted_transport.h"
#include <string.h>
#include <inttypes.h>

/* Control serial adapter (Path B — port adapter, not a contract)
 * Defined in host/port/esp-idf/h_control_serial_adapter.c */
#include "h_control_serial_adapter.h"

static void *g_serial_handle = NULL;

/* ── TLV Helpers ── */

static uint16_t serial_compose_tlv(uint8_t *buf, const uint8_t *data, uint16_t data_length)
{
    const char* ep_name = RPC_EP_NAME_RSP;
    uint16_t ep_length = strlen(ep_name);
    uint16_t count = 0;
    uint8_t idx;

    buf[count] = H_SERIAL_TLV_T_EPNAME;
    count++;
    buf[count] = (ep_length & 0xFF);
    count++;
    buf[count] = ((ep_length >> 8) & 0xFF);
    count++;

    for (idx = 0; idx < ep_length; idx++) {
        buf[count] = ep_name[idx];
        count++;
    }

    buf[count]= H_SERIAL_TLV_T_DATA;
    count++;
    buf[count] = (data_length & 0xFF);
    count++;
    buf[count] = ((data_length >> 8) & 0xFF);
    count++;
    h_memcpy(&buf[count], data, data_length);
    count = count + data_length;
    return count;
}

/* ── Public API ── */

h_err_t h_serial_if_init(void)
{
    if (g_serial_handle) {
        return H_OK;
    }

    g_serial_handle = h_control_serial_drv_open(SERIAL_IF_FILE);
    if (!g_serial_handle) {
        H_LOGE("SERIAL", "h_control_serial_drv_open failed");
        return H_FAIL;
    }

    if (h_control_serial_platform_init() != H_OK) {
        H_LOGE("SERIAL", "h_control_serial_platform_init failed");
        h_control_serial_handle_t* h = (h_control_serial_handle_t*)g_serial_handle;
        h_control_serial_drv_close(&h);
        g_serial_handle = NULL;
        return H_FAIL;
    }

    H_LOGI("SERIAL", "Serial IF initialized");
    return H_OK;
}

bool h_serial_if_is_ready(void)
{
    return (g_serial_handle != NULL);
}

void h_serial_if_deinit(void)
{
    if (g_serial_handle) {
        h_control_serial_platform_deinit();
        h_control_serial_handle_t* h = (h_control_serial_handle_t*)g_serial_handle;
        h_control_serial_drv_close(&h);
        g_serial_handle = NULL;
    }
    H_LOGI("SERIAL", "Serial IF deinitialized");
}

h_err_t h_serial_if_send(const uint8_t *data, uint16_t len)
{
    const char* ep_name = RPC_EP_NAME_RSP;
    int count = 0, out_count = 0;
    uint16_t buf_len = 0;
    uint8_t *write_buf = NULL;

    if (!data || !len || !g_serial_handle) {
        return H_ERR_INVALID_ARG;
    }

    buf_len = H_SERIAL_SIZE_OF_TYPE + H_SERIAL_SIZE_OF_LENGTH + strlen(ep_name) +
              H_SERIAL_SIZE_OF_TYPE + H_SERIAL_SIZE_OF_LENGTH + len;

    write_buf = h_calloc(1, buf_len);
    if (!write_buf) {
        return H_ERR_NO_MEM;
    }

    count = serial_compose_tlv(write_buf, data, len);
    if (!count) {
        h_free(write_buf);
        return H_FAIL;
    }

    if (h_control_serial_drv_write((h_control_serial_handle_t*)g_serial_handle, write_buf, count, &out_count) != H_OK) {
        /* Once h_control_serial_drv_write() is called, write_buf is owned by the serial or
         * transport layer even if the call reports failure. */
        return H_FAIL;
    }

    if (out_count != count) {
        return H_FAIL;
    }

    /* write_buf is freed by the transport layer via callback */
    return H_OK;
}

h_err_t h_serial_if_recv(uint8_t *data, uint16_t *len, int32_t timeout_ms)
{
    (void)timeout_ms;

    if (!data || !len || !g_serial_handle) {
        return H_ERR_INVALID_ARG;
    }

    uint32_t out_nbyte = 0;
    uint8_t *rx_data = h_control_serial_drv_read((h_control_serial_handle_t*)g_serial_handle, &out_nbyte);

    if (!rx_data || out_nbyte == 0) {
        return H_ERR_TIMEOUT;
    }

    if (*len < out_nbyte) {
        h_free(rx_data);
        H_LOGW("SERIAL", "recv buffer too small: need %" PRIu32 ", have %u",
               out_nbyte, *len);
        return H_ERR_INVALID_ARG;
    }

    h_memcpy(data, rx_data, (uint16_t)out_nbyte);
    *len = (uint16_t)out_nbyte;

    h_free(rx_data);
    return H_OK;
}
