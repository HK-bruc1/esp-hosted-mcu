/* host/port/jl/port_init.c
 * JL port lifecycle hooks.
 */

#include "h_init.h"
#include "h_wrapper.h"
#include "h_transport_drv.h"

/* rpc_core.h includes legacy transport_drv.h, which conflicts with the
 * canonical h_transport_drv.h above. Forward-declare only the lifecycle
 * functions we need. */
extern int rpc_core_init(void);
extern int rpc_core_start(void);
extern int rpc_core_stop(void);
extern int rpc_core_deinit(void);

/* bus_handle is owned by host/core/src/h_transport_drv.c. It is left as a
 * non-static global (not an accessor function) so that the port can set the
 * bus handle before the full transport setup is triggered.
 *
 * This is an intentional cross-layer contract: the core owns the variable;
 * the port may write it only during h_port_transport_init().  If upstream
 * ever hides this symbol, the port will need a dedicated core API such as
 * h_transport_init_global(void **bus_handle). */
extern void *bus_handle;

static bool g_rpc_initialized = false;

h_err_t h_port_osal_init(void)  { return H_OK; }
void    h_port_osal_deinit(void) { }

h_err_t h_port_transport_init(void)
{
    /* Only initialize the raw bus handle.  The full transport setup
     * (queue/task creation and slave reconfigure) is performed later by
     * setup_transport() from esp_hosted_api.c, which supplies the upper-layer
     * callback.  Doing the full setup here causes the UART RX task to start
     * before the application has requested a connection. */
    return h_transport_init(&bus_handle);
}

void h_port_transport_deinit(void)
{
    teardown_transport();
}

h_err_t h_port_rpc_init(void)
{
    if (g_rpc_initialized) {
        return H_OK;
    }
    if (rpc_core_init() != H_OK) {
        return H_FAIL;
    }
    if (rpc_core_start() != H_OK) {
        rpc_core_deinit();
        return H_FAIL;
    }
    g_rpc_initialized = true;
    return H_OK;
}

void h_port_rpc_deinit(void)
{
    if (!g_rpc_initialized) {
        return;
    }
    rpc_core_stop();
    rpc_core_deinit();
    g_rpc_initialized = false;
}
