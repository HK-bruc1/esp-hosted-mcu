/* Stubs for serial driver functions required by h_control_serial_adapter.c
 * (moved from host/core/src/h_rpc_slave_if.c to host/port/esp-idf/) in mock builds. */
#include <stdint.h>
#include <stddef.h>
#include "h_types.h"
#include "h_control_serial_adapter.h"

struct serial_handle_s;
typedef struct serial_handle_s serial_ll_handle_t;

serial_ll_handle_t *serial_ll_init(void(*rx_data_ind)(void))
{
    (void)rx_data_ind;
    return NULL;
}

int parse_tlv(const uint8_t *data, size_t len, void *out)
{
    (void)data;
    (void)len;
    (void)out;
    return 0;
}

/* ── h_control_serial_* stubs ── */
static struct h_control_serial_handle { int dummy; } g_mock_handle;

h_control_serial_handle_t *h_control_serial_drv_open(const char *transport)
{
    if (!transport) return NULL;
    return (h_control_serial_handle_t *)&g_mock_handle;
}

int h_control_serial_drv_close(h_control_serial_handle_t **handle)
{
    if (!handle) return H_ERR_INVALID_ARG;
    *handle = NULL;
    return 0;
}

int h_control_serial_drv_write(h_control_serial_handle_t *h,
                                uint8_t *buf, int in_count, int *out_count)
{
    if (!h) return H_ERR_INVALID_ARG;
    if (buf) { /* caller owns buf; mock just consumes it */ }
    if (out_count) *out_count = in_count;
    return 0;
}

uint8_t *h_control_serial_drv_read(h_control_serial_handle_t *h,
                                    uint32_t *out_nbyte)
{
    (void)h;
    if (out_nbyte) *out_nbyte = 0;
    return NULL;
}

int h_control_serial_platform_init(void) { return 0; }
int h_control_serial_platform_deinit(void) { return 0; }
