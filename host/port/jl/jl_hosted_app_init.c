/* host/port/jl/jl_hosted_app_init.c
 *
 * JL (AC701N) application integration entry for ESP-Hosted-MCU Host framework.
 *
 * This file registers an initcall that spawns a dedicated initialization task.
 * The task calls h_hosted_init(), which in turn creates the transport/RPC
 * worker tasks that keep receiving messages from the slave.
 *
 * Design rationale:
 *  - On ESP-IDF, esp_hosted_init() is called synchronously from app_main(),
 *    which is itself a FreeRTOS task; blocking there does not delay system
 *    startup.
 *  - On JL, the application main loop (app_main) runs in the single app_core
 *    task after all initcalls finish. A synchronous h_hosted_init() here would
 *    block UI/power-on until slave reset/handshake completes.
 *  - Therefore JL uses an async init task: register via late_initcall, spawn
 *    jl_hosted_init_task(), and let h_hosted_init() run in its own task context.
 *
 * All platform-specific code stays under host/port/jl/; no upstream application
 * file needs to be modified.
 */

#include "h_port_config.h"
#include "h_init.h"

#include "system/includes.h"
#include "system/init.h"

/* Do NOT include <stdio.h> on JL: include_lib/system/fs/fs.h already defines
 * FILE/fread/fwrite/fseek and conflicts with the C library stdio.h. */
extern int printf(const char *fmt, ...);

#define JL_HOSTED_LOG_TAG  "[ESP_HOSTED]"

static void jl_hosted_init_task(void *priv)
{
    h_err_t ret = h_hosted_init();
    if (ret != H_OK) {
        printf(JL_HOSTED_LOG_TAG " h_hosted_init failed: %d\r\n", (int)ret);
        return;
    }

    printf(JL_HOSTED_LOG_TAG " h_hosted_init OK (%s %s)\r\n",
           H_PORT_NAME, H_PORT_VERSION);
}

static int jl_hosted_app_init(void)
{
    if (os_task_create(jl_hosted_init_task, NULL,
                       H_DEFAULT_TASK_PRIO,
                       H_DEFAULT_TASK_STACK,
                       0, "esp_hosted_init") != 0) {
        printf(JL_HOSTED_LOG_TAG " failed to create init task\r\n");
        return -1;
    }

    return 0;
}

/* Register in late initcall phase:
 *  - RTOS primitives (task, queue, semaphore) are ready.
 *  - Board and driver initcalls have already run.
 *  - The application main loop has not started yet.
 */
late_initcall(jl_hosted_app_init);
