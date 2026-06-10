# Host Port Layer Cleanup — Phase 2 Design

> **Goal**: Upgrade ESP-IDF host platform from A- to A on the portability rubric.
>
> **Scope**: Active source set only. No legacy file deletion. No second-platform PoC.

---

## 1. Current State (Validated 2026-06-10)

| Dimension | Status |
|---|---|
| ESP-IDF active path decoupled from legacy `g_h` / old vtable | **Pass** |
| Legacy symbols (`HOSTED_FREE`, `g_h.funcs`, `->_h_`, etc.) absent from active path | **Pass** |
| Public/active legacy header cleanup | **Pass** |
| Port selector (`ESP_HOSTED_HOST_PORT`) + `port.cmake` entry | **Pass** |
| Three-contract structure + fail-fast validation | **Pass** |
| `h_osal.c` timer is real `esp_timer` adapter | **Pass** |
| `examples/host_compile_check` SPI build | **Pass** |

**Overall grade: A-**. Architecture is sound. Remaining items are polish, not structural.

---

## 2. Remaining Friction Points

### F1: Core layer directly includes ESP-IDF port header (interface coupling)

**Location**: `host/core/src/h_serial_if.c:14`
```c
#include "h_control_serial_adapter.h"  // lives in host/port/esp-idf/
```

**Impact**: Core layer has a compile-time dependency on a file in the ESP-IDF port directory. A non-ESP port must provide this exact header at the same include path, or the core layer won't compile.

**Severity**: Medium. The header itself is platform-neutral (only `stdint.h`), so this is interface coupling, not implementation coupling. But it violates the principle that core should not include headers from the selected port's implementation directory — a non-ESP port must either replicate this header at the same path or the build breaks.

### F2: `esp_hosted_cli.h` hard-depends on `sdkconfig.h`

**Location**: `common/utils/esp_hosted_cli.h:10`
```c
#include "sdkconfig.h"
```

**Impact**: Any file that includes `esp_hosted_cli.h` on a non-ESP platform will fail to compile unless the platform provides a `sdkconfig.h` shim.

**Severity**: Low-medium. The `sdkconfig.h` include is at the top level, but the actual feature gates (`H_ESP_HOSTED_CLI_ENABLED`) are behind `#ifdef CONFIG_ESP_HOSTED_ENABLED` / `#ifdef CONFIG_ESP_HOSTED_COPROCESSOR`, which a non-ESP platform wouldn't define. So the CLI code itself wouldn't be compiled, but the `#include "sdkconfig.h"` would still fail.

### F3: ESP-IDF port bus files include legacy `transport_drv.h`

**Locations** (5 active bus files directly include `transport_drv.h`; `stats.c` uses shared transport definitions via `h_transport_drv.h`):
- `host/port/esp-idf/h_transport_sdio_bus.c:26`
- `host/port/esp-idf/h_transport_uart_bus.c:16`
- `host/port/esp-idf/h_transport_uart.c:12`
- `host/port/esp-idf/h_transport_spi_bus.c:15`
- `host/port/esp-idf/h_transport_spi_hd_bus.c:17`
- `host/port/esp-idf/tools/stats.c:11` (via `h_transport_drv.h`)

**Impact**: These files reuse shared/internal types, macros, and internal APIs from `transport_drv.h` (e.g. `transport_channel_t`, buffer size constants, `chan_arr[]`). This is expected during the transition — the legacy `host/port/esp/freertos/` is NOT in the active source set, but its shared header still provides definitions the bus files need. It means the ESP-IDF port isn't fully self-contained yet.

**Severity**: Low. This is an ESP-IDF-internal concern (port includes its own legacy headers). Doesn't affect a non-ESP port.

### F4: Comment debt in `h_rpc_core.c`

**Location**: `host/core/src/h_rpc_core.c:1197`
```
* Bridges to legacy transport_pserial_* via h_serial_if abstraction.
```

**Impact**: None on code paths. The `transport_pserial_*` symbols are still referenced in `host/drivers/virtual_serial_if/serial_if.c:119` (active compilation), but the comment in core is stale — core no longer calls them directly. This is a core-layer comment debt, not an active coupling.

**Severity**: Trivial.

### F5: `serial_if.c` includes `esp_log.h` directly

**Location**: `host/drivers/virtual_serial_if/serial_if.c:12`
```c
#include "esp_log.h"
```

**Impact**: Driver layer file directly includes ESP-IDF logging header instead of using `h_wrapper.h` log macros.

**Severity**: Low. This is in the driver layer (not core), and `h_wrapper.h` is already included. The `esp_log.h` include is redundant — `DEFINE_LOG_TAG` and `H_LOGE/H_LOGI` are available via `h_wrapper.h`. Can be removed as part of P1 cleanup.

### F6: Naming prefix inconsistency

**Observation**: Core layer includes headers with `esp_` prefix (`esp_hosted_rpc.h`, `esp_hosted_transport.h`, `esp_hosted_bitmasks.h`). These are platform-neutral common headers, but the prefix suggests ESP dependency.

**Impact**: Cosmetic. A non-ESP port developer might initially be confused, but the headers themselves contain only standard C.

**Severity**: Trivial. Renaming would be a large churn for no functional benefit.

---

## 3. Proposed Optimizations

### P1: Extract `h_control_serial_adapter` contract into `host/port/include/` (addresses F1)

**What**: Move the `h_control_serial_adapter.h` header (function declarations only) from `host/port/esp-idf/` to `host/port/include/h_control_serial_contract.h`.

**Why**: Core layer should not include headers from the selected port's implementation directory (`host/port/esp-idf/`). The function declarations are platform-neutral; only the `.c` implementation is ESP-IDF-specific. Core legitimately includes from `host/port/include/`, `host/core/include/`, `common/`, and API public/internal headers — the goal is to eliminate port-implementation-directory leakage, not to restrict include paths to only two directories.

**Consumers** (active source set):
- `host/core/src/h_serial_if.c:14` — core layer (primary fix target)
- `host/drivers/virtual_serial_if/serial_if.c:16` — driver layer (also benefits)
- `host/port/esp-idf/h_control_serial_adapter.c` — the implementation itself
- `tests/stubs/serial_stubs.c`, `tests/test_rpc_bridge.c` — tests

**How**:
1. Create `host/port/include/h_control_serial_contract.h` with the existing function signatures (from `h_control_serial_adapter.h`).
2. Update `host/core/src/h_serial_if.c` to include `h_control_serial_contract.h`.
3. Update `host/drivers/virtual_serial_if/serial_if.c` to include `h_control_serial_contract.h`.
4. Update `host/port/esp-idf/h_control_serial_adapter.c` to include the new contract header.
5. Keep `host/port/esp-idf/h_control_serial_adapter.h` as a thin `#include` redirect for backward compatibility with tests and any external consumers.

**Risk**: Zero. Pure header relocation with no behavioral change.

### P2: Guard `sdkconfig.h` in `esp_hosted_cli.h` (addresses F2)

**What**: Wrap the `#include "sdkconfig.h"` in a `__has_include` guard.

**Why**: Non-ESP platforms shouldn't need to provide a `sdkconfig.h` shim just because they include a common utility header.

**Scope**: Header-only protection. This makes `esp_hosted_cli.h` includable from non-ESP translation units. The `.c` source (`esp_hosted_cli.c`) also directly includes `sdkconfig.h`, `esp_idf_version.h`, and `esp_console.h` — making the CLI *implementation* portable is out of scope for Phase 2.

**How**:
```c
#if defined(__has_include)
#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif
#endif
```

**Risk**: Zero. The downstream feature gates already use `#ifdef CONFIG_ESP_HOSTED_*`, which won't be defined without `sdkconfig.h`.

### P3: Clean up comment debt (addresses F4)

**What**: Update `h_rpc_core.c:1197` comment to reference `h_serial_if` instead of `transport_pserial_*`.

**Risk**: Zero. Comment-only change.

### P4: Document port bus file legacy header dependency (addresses F3, informational)

**What**: Add a comment in each `h_transport_*_bus.c` file explaining why `transport_drv.h` is included and when it can be removed.

**Why**: Future developers need to know this is intentional transition state, not oversight.

**Risk**: Zero. Comment-only.

### P5: Replace `ESP_LOGE` with `H_LOGE` in `serial_if.c` and remove `esp_log.h` (addresses F5)

**What**: In `host/drivers/virtual_serial_if/serial_if.c`:
1. Replace all `ESP_LOGE(TAG, ...)` calls with `H_LOGE(TAG, ...)` (9 occurrences).
2. Replace `ESP_LOGW(TAG, ...)` with `H_LOGW(TAG, ...)` (1 occurrence).
3. Remove `#include "esp_log.h"`.

The file already includes `h_wrapper.h` which provides `H_LOGE/H_LOGW` and `DEFINE_LOG_TAG`.

**Risk**: Zero. `H_LOGE` routes through `g_h_osal.log_write` which is `esp_log_writev` on ESP-IDF — identical behavior.

---

## 4. Out of Scope

- **Deleting legacy files** (`host/port/esp/freertos/`, `host/drivers/rpc/core/`, `host/esp_hosted_os_abstraction.h`). These are compatibility保留, not active coupling.
- **Renaming `esp_*` prefixes** on common headers. Cosmetic churn with no functional benefit.
- **Second-platform PoC**. Separate initiative.
- **Extracting constants/types from `transport_drv.h`** into a smaller internal header. This is an ESP-IDF-internal cleanup that doesn't affect portability.
- **`serial_if.c`'s `#include "h_port_config.h"`** — this is expected behavior for driver-layer code that needs platform config.
- **Replacing `ESP_LOG*` across all `host/drivers/` files** (`power_save_drv.c`, `vhci_drv.c`, `rpc_slave_if.c`, transport leaf drivers, etc.). P5 only targets `serial_if.c`. Full driver-layer log migration is a separate, larger cleanup.

---

## 5. Success Criteria

After implementing P1-P5:

| Criterion | Verification |
|---|---|
| Core layer no longer includes selected port implementation headers | `grep -r '#include.*h_control_serial_adapter' host/core/src/` returns 0 (no port-implementation-directory leakage into core) |
| `esp_hosted_cli.h` header compiles without `sdkconfig.h` | Header-only: `#include "esp_hosted_cli.h"` succeeds in a non-ESP translation unit (`.c` source is out of scope) |
| No stale legacy symbol references in comments | `grep -r 'transport_pserial' host/core/` returns 0 |
| `examples/host_compile_check` SPI build still passes | Build and verify exit code |
| No `esp_log.h` or `ESP_LOG*` in `serial_if.c` | `grep 'esp_log\.h\|ESP_LOG' host/drivers/virtual_serial_if/serial_if.c` returns 0 (other driver files still use `ESP_LOG*` — that's a separate, larger cleanup) |
| Grade upgrade: A- → A | All friction points resolved |

---

## 6. Implementation Order

1. **P1** (highest impact, zero risk) — header relocation
2. **P5** (trivial, bundled with P1) — remove redundant `esp_log.h`
3. **P2** (defensive, zero risk) — `sdkconfig.h` guard
4. **P3** (trivial) — comment fix
5. **P4** (trivial) — comment documentation

Total estimated effort: < 1 hour. All changes are mechanical with no behavioral impact.
