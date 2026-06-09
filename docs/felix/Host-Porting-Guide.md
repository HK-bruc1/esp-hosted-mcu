# Host Porting Guide

## Overview

ESP-Hosted Host uses a contract-based architecture to separate platform-independent logic from platform-specific code. The **core layer** (`host/core/src/`) contains all protocol, RPC, and Wi-Fi logic. It communicates with the platform through three global const vtable structs defined in `host/port/include/h_port_contract.h`. To port to a new platform, you implement these three vtables and four lifecycle functions -- nothing else.

```
 Application
      |
 [Core Layer]  host/core/src/          (platform-independent)
      |
 [Wrapper]     host/port/include/h_wrapper.h   (h_malloc, h_transmit, H_LOGE, ...)
      |
 [Contracts]   host/port/include/h_port_contract.h  (h_osal_contract_t, h_event_contract_t, h_transport_contract_t)
      |
 [Your Port]   host/port/<your-platform>/       (platform-specific)
```

Core code never calls platform APIs directly. It calls wrapper macros (`h_malloc()`, `h_mutex_lock()`, `h_transmit()`, `H_LOGE()`, ...) which expand to `g_h_osal.func(...)`, `g_h_transport.func(...)`, etc. Your port defines these three globals.

## What You Need to Implement

### Minimum files

| # | File | Purpose |
|---|------|---------|
| 1 | `h_port_config.h` | Platform config: transport selection, feature flags, thread defaults |
| 2 | `h_osal.c` | Implement `g_h_osal` (OSAL contract) |
| 3 | `h_event.c` | Implement `g_h_event` (event contract) |
| 4 | `h_transport_*.c` | Implement `g_h_transport` (transport HAL for your bus) |
| 5 | `port_init.c` | Implement the 4 `h_port_*` lifecycle init/deinit pairs |
| 6 | `port.cmake` | Build system integration |

### What NOT to implement

- `hosted_osi_funcs_t` / `g_h` / `g_hosted_osi_funcs` -- this is a deprecated legacy vtable from the old architecture. Ignore it.
- `esp_hosted_os_abstraction.h` -- deprecated header, do not include or implement.

## Contract Reference

### h_osal_contract_t (`g_h_osal`)

OS abstraction layer. Define as `const h_osal_contract_t g_h_osal = { ... };` in your `h_osal.c`.

**Required function pointers:**

| Category | Function | Signature |
|----------|----------|-----------|
| Memory | `malloc` | `void* (*)(size_t)` |
| Memory | `calloc` | `void* (*)(size_t, size_t)` |
| Memory | `realloc` | `void* (*)(void*, size_t)` |
| Memory | `free` | `void (*)(void*)` |
| Memory | `memcpy` | `void* (*)(void*, const void*, size_t)` |
| Memory | `memset` | `void* (*)(void*, int, size_t)` |
| Memory | `malloc_align` | `void* (*)(size_t size, size_t align)` |
| Memory | `free_align` | `void (*)(void*)` |
| Threads | `thread_create` | `int (*)(const char *name, uint32_t prio, uint32_t stack, void (*fn)(void*), void *arg, h_thread_t *out)` |
| Threads | `thread_delete` | `int (*)(h_thread_t)` |
| Mutex | `mutex_create` | `int (*)(h_mutex_t *out)` |
| Mutex | `mutex_lock` | `int (*)(h_mutex_t, int32_t timeout_ms)` |
| Mutex | `mutex_unlock` | `int (*)(h_mutex_t)` |
| Mutex | `mutex_delete` | `int (*)(h_mutex_t)` |
| Queue | `queue_create` | `int (*)(uint32_t count, uint32_t item_size, h_queue_t *out)` |
| Queue | `queue_send` | `int (*)(h_queue_t, const void *item, int32_t timeout_ms)` |
| Queue | `queue_recv` | `int (*)(h_queue_t, void *item, int32_t timeout_ms)` |
| Queue | `queue_msg_waiting` | `int (*)(h_queue_t)` |
| Queue | `queue_reset` | `int (*)(h_queue_t)` |
| Queue | `queue_delete` | `int (*)(h_queue_t)` |
| Semaphore | `sem_create` | `int (*)(uint32_t max, uint32_t init, h_semaphore_t *out)` |
| Semaphore | `sem_take` | `int (*)(h_semaphore_t, int32_t timeout_ms)` |
| Semaphore | `sem_give` | `int (*)(h_semaphore_t)` |
| Semaphore | `sem_give_from_isr` | `int (*)(h_semaphore_t, void *isr_ctx)` |
| Semaphore | `sem_delete` | `int (*)(h_semaphore_t)` |
| Critical | `enter_critical` | `void (*)(void)` |
| Critical | `exit_critical` | `void (*)(void)` |
| Timer | `timer_create` | `int (*)(const char *name, h_timer_t *out)` |
| Timer | `timer_start` | `int (*)(h_timer_t, uint32_t period_ms, bool periodic, void (*cb)(void*), void *arg)` |
| Timer | `timer_stop` | `int (*)(h_timer_t)` -- stops AND frees handle |
| Timer | `timer_delete` | `int (*)(h_timer_t)` -- cleanup for never-started timers |
| Timer | `get_time_ms` | `uint64_t (*)(void)` |
| Delay | `msleep` | `void (*)(uint32_t ms)` |
| Delay | `usleep` | `void (*)(uint32_t us)` |
| Delay | `blocking_delay` | `void (*)(unsigned int iterations)` |
| Logging | `log_write` | `void (*)(int level, const char *tag, const char *fmt, ...)` |

**Optional function pointers (leave NULL if not needed):**

| Function | Purpose |
|----------|---------|
| `restart_host` | Reboot the host MCU |
| `hosted_init_hook` | Called after core init completes |
| `woke_from_ps` | Check if woke from power-save |
| `ps_init` | Initialize power-save |
| `spi_hd_set_data_lines` | SPI-HD data line width switch |
| `get_host_wakeup_or_reboot_reason` | Power-save wakeup reason |
| `config_host_power_save_hal` | Configure power-save HAL |
| `start_host_power_save_hal` | Start power-save mode |

**Timer lifecycle note:** `timer_stop()` stops the timer AND frees the handle. Callers set their handle to NULL after stop. `timer_delete()` is for handles that were created but never started, or error-path cleanup after create.

**ISR note:** If your platform has no ISR context, implement `sem_give_from_isr` as a simple call to your normal `sem_give`.

### h_event_contract_t (`g_h_event`)

Event dispatch layer. Define as `const h_event_contract_t g_h_event = { ... };` in your `h_event.c`.

| Function | Signature | Description |
|----------|-----------|-------------|
| `register_handler` | `int (*)(h_event_base_t base, int32_t event_id, h_event_handler_t handler, void *user_ctx)` | Register callback for event |
| `unregister_handler` | `int (*)(h_event_base_t base, int32_t event_id, h_event_handler_t handler)` | Remove callback |
| `post` | `int (*)(h_event_base_t base, int32_t event_id, void *event_data, size_t event_data_size)` | Post event to registered handlers |
| `wifi_post` | `int (*)(int32_t event_id, void *event_data, size_t event_data_size, int32_t timeout_ms)` | Convenience: post to H_EVENT_WIFI base |

Event bases are defined in `h_types.h`: `H_EVENT_WIFI`, `H_EVENT_IP`, `H_EVENT_HOSTED`.

### h_transport_contract_t (`g_h_transport`)

Transport HAL. Define as `const h_transport_contract_t g_h_transport = { ... };` in your transport file. One vtable covers all transport types -- fill the fields for your bus, leave the rest NULL.

**Base (always required):**

| Function | Signature |
|----------|-----------|
| `init` | `int (*)(void **out_handle)` |
| `deinit` | `int (*)(void *handle)` |
| `bus_ready` | `int (*)(void *handle)` |
| `transmit` | `int (*)(uint8_t if_type, uint8_t if_num, uint8_t *payload, uint16_t len, uint8_t zcopy, void *to_free, void (*free_fn)(void*), uint8_t flags)` |

**SPI-specific (required when `H_TRANSPORT_IN_USE == H_TRANSPORT_SPI`):**

| Function | Signature |
|----------|-----------|
| `spi_transfer` | `int (*)(void *handle, void *transfer_ctx)` |
| `gpio_config` | `int (*)(uint32_t pin, uint32_t mode)` |
| `gpio_set_intr` | `int (*)(uint32_t pin, uint32_t intr_type, void (*isr)(void*), void *arg)` |

**SPI-HD-specific (required when `H_TRANSPORT_IN_USE == H_TRANSPORT_SPI_HD`):**

| Function | Signature |
|----------|-----------|
| `spi_hd_read_reg` | `int (*)(void *handle, uint32_t reg, uint32_t *data, int poll, bool lock)` |
| `spi_hd_write_reg` | `int (*)(void *handle, uint32_t reg, uint32_t *data, bool lock)` |
| `spi_hd_read_dma` | `int (*)(void *handle, uint8_t *data, uint16_t size, bool lock)` |
| `spi_hd_write_dma` | `int (*)(void *handle, uint8_t *data, uint16_t size, bool lock)` |
| `spi_hd_send_cmd9` | `int (*)(void *handle)` |
| `gpio_config` | (same as SPI) |
| `gpio_set_intr` | (same as SPI) |

**SDIO-specific (required when `H_TRANSPORT_IN_USE == H_TRANSPORT_SDIO`):**

| Function | Signature |
|----------|-----------|
| `sdio_card_init` | `int (*)(void *handle, bool show_config)` |
| `sdio_read_reg` | `int (*)(void *handle, uint32_t reg, uint8_t *data, uint16_t size, bool lock)` |
| `sdio_write_reg` | `int (*)(void *handle, uint32_t reg, uint8_t *data, uint16_t size, bool lock)` |
| `sdio_read_block` | `int (*)(void *handle, uint32_t reg, uint8_t *data, uint16_t size, bool lock)` |
| `sdio_write_block` | `int (*)(void *handle, uint32_t reg, uint8_t *data, uint16_t size, bool lock)` |
| `sdio_wait_intr` | `int (*)(void *handle, uint32_t timeout_ms)` |
| `gpio_config` | (same as SPI) |
| `gpio_write` | `int (*)(uint32_t pin, uint32_t value)` |

**UART-specific (required when `H_TRANSPORT_IN_USE == H_TRANSPORT_UART`):**

| Function | Signature |
|----------|-----------|
| `uart_read` | `int (*)(void *handle, uint8_t *data, uint16_t size)` |
| `uart_write` | `int (*)(void *handle, uint8_t *data, uint16_t size)` |
| `uart_flush` | `int (*)(void *handle)` |
| `gpio_config` | (same as SPI) |
| `gpio_write` | (same as SDIO) |

**Optional (leave NULL if not needed):**

| Function | Purpose |
|----------|---------|
| `gpio_pull` | Configure pin pull-up/pull-down |
| `gpio_hold` | Hold pin state during sleep |
| `netif_create` | Create network interface |
| `netif_destroy` | Destroy network interface |
| `gpio_clear_intr` | Clear GPIO interrupt |
| `gpio_read` | Read GPIO pin value |

## Implementation Guide

### Step 1: Create port directory

```
host/port/<your-platform>/
```

For a platform named "myrtos", create `host/port/myrtos/`.

### Step 2: Write h_port_config.h

This header defines platform identity and compile-time configuration. Include it from `host/port/include/h_config.h` via the `#include_next` mechanism, or define `H_PORT_NAME` before the fallback triggers.

Required macros:

```c
/* Transport selection -- exactly one */
#define H_TRANSPORT_IN_USE  H_TRANSPORT_SPI   /* or H_TRANSPORT_SDIO, H_TRANSPORT_SPI_HD, H_TRANSPORT_UART */

/* Platform identity (logged at init) */
#define H_PORT_NAME         "myrtos"
#define H_PORT_VERSION      "1.0.0"
#define H_PORT_RTOS         "MyRTOS"
#define H_PORT_RTOS_VER     "1.0"
#define H_PORT_CHIP         "my-mcu"
#define H_PORT_BUILD_DATE   __DATE__

/* Thread defaults (override if needed) */
#define H_DEFAULT_TASK_STACK  4096
#define H_DEFAULT_TASK_PRIO   5

/* Transport buffer size */
#define H_MAX_TRANSPORT_BUFFER_SIZE  1600

/* Feature flags */
#define H_FEATURE_BLUETOOTH  0
#define H_FEATURE_OTA        0
```

Transport constants are defined in `host/port/include/h_config.h`:
- `H_TRANSPORT_NONE` = 0
- `H_TRANSPORT_SDIO` = 1
- `H_TRANSPORT_SPI_HD` = 2
- `H_TRANSPORT_SPI` = 3
- `H_TRANSPORT_UART` = 4

### Step 3: Implement h_osal.c

Pattern: write adapter functions that wrap your RTOS primitives, then define the vtable.

```c
#include "h_port_contract.h"
#include "h_port_config.h"
#include <myrtos/api.h>

static int my_mutex_create(h_mutex_t *out) {
    my_mutex_t *m = my_malloc(sizeof(my_mutex_t));
    if (!m) return H_ERR_NO_MEM;
    my_mutex_init(m);
    *out = (h_mutex_t)m;
    return H_OK;
}

/* ... other adapter functions ... */

static void my_log_write(int level, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    my_platform_log(level, tag, fmt, args);
    va_end(args);
}

const h_osal_contract_t g_h_osal = {
    .malloc       = my_malloc,
    .calloc       = my_calloc,
    .realloc      = my_realloc,
    .free         = my_free,
    .memcpy       = my_memcpy,
    .memset       = my_memset,
    .malloc_align = my_malloc_align,
    .free_align   = my_free_align,
    .thread_create = my_thread_create,
    /* ... fill all required slots ... */
    .log_write    = my_log_write,
    /* Optional slots can be NULL */
    .restart_host = NULL,
};
```

For `sem_give_from_isr`: if your platform has no ISR context, delegate to the normal give:

```c
static int my_sem_give_from_isr(h_semaphore_t sem, void *isr_ctx) {
    (void)isr_ctx;
    return my_sem_give(sem);
}
```

### Step 4: Implement h_event.c

Pattern: implement a handler registry (linked list or array) and dispatch on post.

```c
const h_event_contract_t g_h_event = {
    .register_handler   = my_event_register,
    .unregister_handler = my_event_unregister,
    .post               = my_event_post,
    .wifi_post          = my_event_wifi_post,
};
```

`wifi_post` is a convenience wrapper that calls `post` with `H_EVENT_WIFI` as the base. The `timeout_ms` parameter maps to your event queue's blocking semantics.

### Step 5: Implement transport

Create `h_transport_<bus>.c` (e.g. `h_transport_spi.c`). Define the vtable with only the fields relevant to your bus:

```c
const h_transport_contract_t g_h_transport = {
    .init         = my_bus_init,
    .deinit       = my_bus_deinit,
    .bus_ready    = my_bus_ready,
    .transmit     = my_bus_transmit,
    .spi_transfer = my_spi_transfer,
    .gpio_config  = my_gpio_config,
    .gpio_set_intr = my_gpio_set_intr,
    /* All other fields zero/NULL */
};
```

The runtime validates that all required fields for `H_TRANSPORT_IN_USE` are non-NULL at startup (see `h_validate_contracts()` in `h_init.c`).

### Step 6: Implement port_init.c

Define the four lifecycle init/deinit pairs. These are called by `h_hosted_init()` in strict order with rollback on failure.

```c
#include "h_port_contract.h"
#include "h_port_config.h"

/* OSAL: bootstrap your RTOS if not already running */
h_err_t h_port_osal_init(void) {
    /* If RTOS is already running (e.g. called from app_main), return H_OK */
    return H_OK;
}
void h_port_osal_deinit(void) {}

/* Event: create default event loop if needed */
h_err_t h_port_event_init(void) {
    return H_OK;
}
void h_port_event_deinit(void) {}

/* Transport: cross-cutting bus setup (optional) */
h_err_t h_port_transport_init(void) {
    return H_OK;
}
void h_port_transport_deinit(void) {}

/* RPC: init and start the RPC core */
extern int rpc_core_init(void);
extern int rpc_core_deinit(void);
extern int rpc_core_start(void);
extern int rpc_core_stop(void);

h_err_t h_port_rpc_init(void) {
    if (rpc_core_init() != 0) return H_FAIL;
    if (rpc_core_start() != 0) return H_FAIL;
    return H_OK;
}
void h_port_rpc_deinit(void) {
    rpc_core_stop();
    rpc_core_deinit();
}
```

The init order is: osal -> event -> transport -> rpc. Deinit runs in reverse. If any init fails, all previously initialized layers are torn down.

### Step 7: Write port.cmake

Export two variables for the build system:

```cmake
# Port source files
set(ESP_HOSTED_PORT_SRCS
    "${host_dir}/port/myrtos/port_init.c"
    "${host_dir}/port/myrtos/h_osal.c"
    "${host_dir}/port/myrtos/h_event.c"
    "${host_dir}/port/myrtos/h_transport_spi.c"
)

# Port-specific private include directories
set(ESP_HOSTED_PORT_PRIV_INCLUDE_DIRS
    "${host_dir}/port/myrtos"
)
```

Conditionally add transport sources if your port supports multiple buses:

```cmake
if(MY_TRANSPORT STREQUAL "sdio")
    list(APPEND ESP_HOSTED_PORT_SRCS "${host_dir}/port/myrtos/h_transport_sdio.c")
elseif(MY_TRANSPORT STREQUAL "spi")
    list(APPEND ESP_HOSTED_PORT_SRCS "${host_dir}/port/myrtos/h_transport_spi.c")
endif()
```

## Reference Implementations

### ESP-IDF port (production reference)

`host/port/esp-idf/` -- complete implementation for ESP32 with FreeRTOS. Covers all four transports (SPI, SPI-HD, SDIO, UART). This is the primary reference for a production-quality port.

Key files:
- `h_osal.c` -- maps to FreeRTOS primitives
- `h_event.c` -- wraps `esp_event` system
- `h_transport_*.c` -- one file per transport, plus bus-level driver files
- `port_init.c` -- lifecycle hooks
- `port.cmake` -- conditional transport source inclusion

### Linux mock port (test reference)

`host/port/linux/` -- POSIX/pthread implementation, approximately 640 lines total. Useful as a minimal template. Implements only SPI transport as a mock stub. Source files are in `host/port/linux/src/`.

Key files:
- `h_port_config.h` -- root-level config
- `src/h_osal.c` -- pthread-based OSAL (~430 lines)
- `src/h_event.c` -- linked-list event registry (~107 lines)
- `src/h_transport_mock.c` -- SPI transport stubs (~96 lines)

## Validation

### Core isolation check

Verifies that core layer source files do not include platform-specific headers:

```bash
bash scripts/check_core_isolation.sh
```

### Port surface check

Verifies that all required contract symbols are exported by the port:

```bash
bash scripts/check_host_port_surface.sh
```

### Runtime validation

`h_hosted_init()` (in `host/core/src/h_init.c`) calls `h_validate_contracts()` which checks that all required function pointers in the three vtables are non-NULL. If any required slot is missing, init fails with `H_ERR_INVALID_ARG` and logs which contract is incomplete.

Bus-specific validation is compile-time gated on `H_TRANSPORT_IN_USE` -- the runtime only checks the fields relevant to your selected transport.
