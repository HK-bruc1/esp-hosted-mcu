# Host Port Layer Cleanup — Phase 2 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Upgrade ESP-IDF host platform from A- to A on the portability rubric by eliminating remaining port-implementation-directory leakage from core layer and hardening headers for non-ESP includability.

**Architecture:** Five independent, zero-risk mechanical changes: (1) relocate serial adapter contract header to `host/port/include/`, (2) replace `ESP_LOG*` with `H_LOG*` in `serial_if.c`, (3) guard `sdkconfig.h` in CLI header, (4) fix stale comment in RPC core, (5) document legacy header dependency in port bus files.

**Tech Stack:** C, ESP-IDF, pre-commit hooks

---

### Task 1: Extract `h_control_serial_adapter` contract header (P1 + P5)

**Files:**
- Create: `host/port/include/h_control_serial_contract.h`
- Modify: `host/core/src/h_serial_if.c:12-14`
- Modify: `host/drivers/virtual_serial_if/serial_if.c:12,14-16`
- Modify: `host/port/esp-idf/h_control_serial_adapter.c` (include redirect)
- Modify: `host/port/esp-idf/h_control_serial_adapter.h` (thin redirect)

- [ ] **Step 1: Create the contract header at `host/port/include/h_control_serial_contract.h`**

```c
/* host/port/include/h_control_serial_contract.h
 *
 * Control Serial Adapter — platform-neutral contract.
 *
 * Declares the function signatures that the control serial adapter must
 * implement. Port layers provide the actual implementation (e.g.
 * host/port/esp-idf/h_control_serial_adapter.c).
 *
 * Core layer and driver layer include THIS header, not the port-specific
 * adapter header. */

#ifndef H_CONTROL_SERIAL_CONTRACT_H
#define H_CONTROL_SERIAL_CONTRACT_H

#include <stdint.h>

/* Opaque handle — callers only pass the pointer through */
struct h_control_serial_handle;
typedef struct h_control_serial_handle h_control_serial_handle_t;

h_control_serial_handle_t *h_control_serial_drv_open (const char *transport);
int h_control_serial_drv_close(h_control_serial_handle_t **handle);
int h_control_serial_drv_write(h_control_serial_handle_t *handle,
                                uint8_t *buf, int in_count, int *out_count);
uint8_t *h_control_serial_drv_read (h_control_serial_handle_t *handle,
                                     uint32_t *out_nbyte);
int h_control_serial_platform_init  (void);
int h_control_serial_platform_deinit(void);

#endif /* H_CONTROL_SERIAL_CONTRACT_H */
```

- [ ] **Step 2: Update `host/port/esp-idf/h_control_serial_adapter.h` to be a thin redirect**

Replace the entire file content with:

```c
/* host/port/esp-idf/h_control_serial_adapter.h
 *
 * Backward-compatibility redirect. New code should include
 * h_control_serial_contract.h from host/port/include/ instead. */

#ifndef H_CONTROL_SERIAL_ADAPTER_H
#define H_CONTROL_SERIAL_ADAPTER_H

#include "h_control_serial_contract.h"

#endif /* H_CONTROL_SERIAL_ADAPTER_H */
```

- [ ] **Step 3: Update `host/core/src/h_serial_if.c` to include the contract header**

Change lines 12-14 from:
```c
/* Control serial adapter (Path B — port adapter, not a contract)
 * Defined in host/port/esp-idf/h_control_serial_adapter.c */
#include "h_control_serial_adapter.h"
```
to:
```c
/* Control serial adapter contract — platform-neutral declarations.
 * ESP-IDF implementation in host/port/esp-idf/h_control_serial_adapter.c */
#include "h_control_serial_contract.h"
```

- [ ] **Step 4: Update `host/drivers/virtual_serial_if/serial_if.c` — fix include and replace ESP_LOG***

First, change lines 12-16 from:
```c
#include "esp_log.h"

/* Control serial adapter (Path B — port adapter, not a contract)
 * Defined in host/port/esp-idf/h_control_serial_adapter.c */
#include "h_control_serial_adapter.h"
```
to:
```c
/* Control serial adapter contract — platform-neutral declarations.
 * ESP-IDF implementation in host/port/esp-idf/h_control_serial_adapter.c */
#include "h_control_serial_contract.h"
```

Then replace all `ESP_LOGE(TAG, ...)` with `H_LOGE(TAG, ...)` and `ESP_LOGW(TAG, ...)` with `H_LOGW(TAG, ...)` in the file. There are 10 occurrences:
- Line 100: `ESP_LOGE(TAG, "Data Type not matched...` → `H_LOGE(TAG, "Data Type not matched...`
- Line 104: `ESP_LOGE(TAG, "Endpoint Name not matched...` → `H_LOGE(TAG, "Endpoint Name not matched...`
- Line 108: `ESP_LOGE(TAG, "Endpoint length not matched...` → `H_LOGE(TAG, "Endpoint length not matched...`
- Line 113: `ESP_LOGE(TAG, "Endpoint type not matched...` → `H_LOGE(TAG, "Endpoint type not matched...`
- Line 125: `ESP_LOGE(TAG, "Platform deinit failed\n")` → `H_LOGE(TAG, "Platform deinit failed\n")`
- Line 131: `ESP_LOGE(TAG, "Failed to close driver interface\n")` → `H_LOGE(TAG, "Failed to close driver interface\n")`
- Line 172: `ESP_LOGW(TAG, "Empty RPC data, ignored")` → `H_LOGW(TAG, "Empty RPC data, ignored")`
- Line 197: `ESP_LOGE(TAG, "Serial connection closed?\n")` → `H_LOGE(TAG, "Serial connection closed?\n")`
- Line 203: `ESP_LOGE(TAG, "Failed to compose TX data\n")` → `H_LOGE(TAG, "Failed to compose TX data\n")`
- Line 209: `ESP_LOGE(TAG, "Failed to write TX data\n")` → `H_LOGE(TAG, "Failed to write TX data\n")`

- [ ] **Step 5: Verify no core-layer leakage remains**

Run: `grep -r '#include.*h_control_serial_adapter' host/core/src/`
Expected: 0 matches

Run: `grep 'esp_log\.h\|ESP_LOG' host/drivers/virtual_serial_if/serial_if.c`
Expected: 0 matches

- [ ] **Step 6: Run pre-commit checks**

Run: `pre-commit run --all-files`
Expected: all checks pass (version sync, RPC consistency, weak functions, changelog)

- [ ] **Step 7: Commit**

```bash
git add host/port/include/h_control_serial_contract.h \
        host/port/esp-idf/h_control_serial_adapter.h \
        host/core/src/h_serial_if.c \
        host/drivers/virtual_serial_if/serial_if.c
git commit -m "refactor(host): extract serial adapter contract to port/include

Move h_control_serial_adapter.h function declarations to
host/port/include/h_control_serial_contract.h so core layer no longer
includes headers from the selected port's implementation directory.
Replace ESP_LOG* with H_LOG* in serial_if.c to remove direct esp_log.h
dependency from the driver layer."
```

---

### Task 2: Guard `sdkconfig.h` in `esp_hosted_cli.h` (P2)

**Files:**
- Modify: `common/utils/esp_hosted_cli.h:10`

- [ ] **Step 1: Wrap `sdkconfig.h` include with `__has_include` guard**

Change line 10 from:
```c
#include "sdkconfig.h"
```
to:
```c
#if defined(__has_include)
#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif
#endif
```

- [ ] **Step 2: Verify the header is structurally sound**

Run: `grep -A5 '__has_include' common/utils/esp_hosted_cli.h`
Expected: shows the three-line guard wrapping `#include "sdkconfig.h"`

- [ ] **Step 3: Commit**

```bash
git add common/utils/esp_hosted_cli.h
git commit -m "fix(common): guard sdkconfig.h in esp_hosted_cli.h

Wrap #include \"sdkconfig.h\" in __has_include guard so non-ESP
platforms can include the CLI header without providing a sdkconfig.h
shim. The .c implementation remains ESP-IDF-only (out of scope)."
```

---

### Task 3: Fix stale comment in `h_rpc_core.c` (P3)

**Files:**
- Modify: `host/core/src/h_rpc_core.c:1197`

- [ ] **Step 1: Update the stale comment**

Change line 1197 from:
```c
 * Bridges to legacy transport_pserial_* via h_serial_if abstraction.
```
to:
```c
 * Bridges to h_serial_if abstraction (serial adapter contract).
```

- [ ] **Step 2: Verify no other `transport_pserial` references remain in core**

Run: `grep -r 'transport_pserial' host/core/`
Expected: 0 matches

- [ ] **Step 3: Commit**

```bash
git add host/core/src/h_rpc_core.c
git commit -m "docs(host): fix stale transport_pserial comment in h_rpc_core.c

The comment referenced legacy transport_pserial_* symbols that core
no longer calls. Updated to reference h_serial_if abstraction."
```

---

### Task 4: Document port bus file legacy header dependency (P4)

**Files:**
- Modify: `host/port/esp-idf/h_transport_sdio_bus.c:26`
- Modify: `host/port/esp-idf/h_transport_spi_bus.c:15`
- Modify: `host/port/esp-idf/h_transport_spi_hd_bus.c:17`
- Modify: `host/port/esp-idf/h_transport_uart.c:12`
- Modify: `host/port/esp-idf/h_transport_uart_bus.c:16`
- Modify: `host/port/esp-idf/tools/stats.c:11`

- [ ] **Step 1: Add transition comment to `h_transport_sdio_bus.c`**

Add above line 26 (`#include "transport_drv.h"`):
```c
/* Transition: reuse shared types/macros (transport_channel_t, chan_arr[],
 * buffer size constants) from legacy transport_drv.h. Can be removed
 * when ESP-IDF port is fully self-contained. */
```

- [ ] **Step 2: Add transition comment to `h_transport_spi_bus.c`**

Add above line 15 (`#include "transport_drv.h"`):
```c
/* Transition: reuse shared types/macros from legacy transport_drv.h.
 * Can be removed when ESP-IDF port is fully self-contained. */
```

- [ ] **Step 3: Add transition comment to `h_transport_spi_hd_bus.c`**

Add above line 17 (`#include "transport_drv.h"`):
```c
/* Transition: reuse shared types/macros from legacy transport_drv.h.
 * Can be removed when ESP-IDF port is fully self-contained. */
```

- [ ] **Step 4: Add transition comment to `h_transport_uart.c`**

Add above line 12 (`#include "transport_drv.h"`):
```c
/* Transition: reuse shared types/macros from legacy transport_drv.h.
 * Can be removed when ESP-IDF port is fully self-contained. */
```

- [ ] **Step 5: Add transition comment to `h_transport_uart_bus.c`**

Add above line 16 (`#include "transport_drv.h"`):
```c
/* Transition: reuse shared types/macros from legacy transport_drv.h.
 * Can be removed when ESP-IDF port is fully self-contained. */
```

- [ ] **Step 6: Add transition comment to `tools/stats.c`**

Add above line 11 (`#include "h_transport_drv.h"`):
```c
/* Transition: uses shared transport definitions via h_transport_drv.h.
 * Can be removed when ESP-IDF port is fully self-contained. */
```

- [ ] **Step 7: Commit**

```bash
git add host/port/esp-idf/h_transport_sdio_bus.c \
        host/port/esp-idf/h_transport_spi_bus.c \
        host/port/esp-idf/h_transport_spi_hd_bus.c \
        host/port/esp-idf/h_transport_uart.c \
        host/port/esp-idf/h_transport_uart_bus.c \
        host/port/esp-idf/tools/stats.c
git commit -m "docs(host): document legacy transport_drv.h dependency in port bus files

Add transition comments explaining why these files include transport_drv.h
and when the dependency can be removed."
```

---

### Task 5: Final verification

- [ ] **Step 1: Run all success criteria checks**

```bash
# Core no longer includes port implementation headers
grep -r '#include.*h_control_serial_adapter' host/core/src/
# Expected: 0 matches

# CLI header compiles without sdkconfig.h (structural check)
grep -c '__has_include' common/utils/esp_hosted_cli.h
# Expected: 1 match

# No stale transport_pserial in core
grep -r 'transport_pserial' host/core/
# Expected: 0 matches

# serial_if.c has no ESP_LOG
grep 'esp_log\.h\|ESP_LOG' host/drivers/virtual_serial_if/serial_if.c
# Expected: 0 matches
```

- [ ] **Step 2: Run `pre-commit run --all-files`**

Expected: all checks pass

- [ ] **Step 3: Run `examples/host_compile_check` SPI build (if toolchain available)**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py build
```

Expected: build succeeds (only `ESP_ROM_ELF_DIR` gdbinit warning, no errors)
