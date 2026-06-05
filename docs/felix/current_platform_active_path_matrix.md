# Current Platform Active Path Matrix

> WP 0 of `23.ESP32平台完全解耦与移植友好型收尾方案.md`
> Date: 2026-06-05
> Control plane closure path: **B (current-platform adapter closure)**

## 1. Classification Key

| Classification | Meaning |
|---|---|
| `required-active` | Always compiled; must pass all legacy checks (no allowlist) |
| `conditional-active` | Compiled when Kconfig enabled; must pass when enabled |
| `legacy-helper` | Short-term retention; must have owner, reason, and exit condition |

## 2. Active Source Set per Transport

### Always-compiled (all transports)

| Layer | File | Classification | Notes |
|---|---|---|---|
| API | `host/api/src/esp_wifi_weak.c` | required-active | Weak function bridge |
| API | `host/api/src/esp_hosted_api.c` | required-active | `#if 0` dead code — WP 1 target |
| API | `host/api/src/esp_hosted_transport_config.c` | required-active | Clean |
| API | `host/api/src/esp_hosted_ota_api.c` | required-active | Clean |
| Core | `host/core/src/h_init.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_api.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_event.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_serial_if.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_transport_drv.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_transport_util.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_rpc_core.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_rpc_req.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_rpc_rsp.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_rpc_evt.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_rpc_utils.c` | required-active | 12/12 isolation |
| Core | `host/core/src/h_rpc_wrap.c` | required-active | 12/12 isolation |
| RPC driver | `host/drivers/rpc/slaveif/rpc_slave_if.c` | required-active | Adapter |
| Serial LL | `host/drivers/serial/serial_ll_if.c` | required-active | Control adapter; `esp_hosted_tx()` in WP 1 scope |
| Virtual serial | `host/drivers/virtual_serial_if/serial_if.c` | required-active | Adapter |
| Stats | `host/port/esp-idf/tools/stats.c` | required-active | WP 1 target |
| Power save | `host/drivers/power_save/power_save_drv.c` | required-active | WP 3b target; no CMake Kconfig guard |
| CLI | `common/utils/esp_hosted_cli.c` | required-active | Utility |
| New port | `host/port/esp-idf/port_init.c` | required-active | Adapter |
| New port | `host/port/esp-idf/h_osal.c` | required-active | Adapter; timer stub — WP 3a target |
| New port | `host/port/esp-idf/h_event.c` | required-active | Adapter |
| New port | `host/port/esp-idf/h_wifi_type_adapt.c` | required-active | Adapter |
| New port | `host/port/esp-idf/h_control_serial_adapter.c` | required-active | Control adapter |
| Legacy port | `host/port/esp/freertos/src/port_esp_hosted_host_os.c` | legacy-helper | Defines `g_h`; retains native helpers; WP 4 target |
| Legacy port | `host/port/esp/freertos/src/port_esp_hosted_host_transport_defaults.c` | legacy-helper | `#ifdef` transport defaults; no legacy vtable calls; WP 4 inventory |

### Conditional-active (Kconfig-gated)

| Kconfig Gate | File | Notes |
|---|---|---|
| `CONFIG_ESP_HOSTED_NIMBLE_HCI_VHCI` or `CONFIG_ESP_HOSTED_BLUEDROID_HCI_VHCI` | `host/drivers/bt/vhci_drv.c` | WP 2 target |

### Transport-specific

| Transport | Driver | New Port | Legacy Port |
|---|---|---|---|
| SPI | `host/drivers/transport/spi/spi_drv.c` | `host/port/esp-idf/h_transport_spi.c` | `host/port/esp/freertos/src/port_esp_hosted_host_spi.c` |
| SPI-HD | `host/drivers/transport/spi_hd/spi_hd_drv.c` | `host/port/esp-idf/h_transport_spi_hd.c` | `host/port/esp/freertos/src/port_esp_hosted_host_spi_hd.c` |
| SDIO | `host/drivers/transport/sdio/sdio_drv.c` | `host/port/esp-idf/h_transport_sdio.c` | `host/port/esp/freertos/src/port_esp_hosted_host_sdio.c` |
| UART | `host/drivers/transport/uart/uart_drv.c` | `host/port/esp-idf/h_transport_uart.c` | `host/port/esp/freertos/src/port_esp_hosted_host_uart.c` |

## 3. Legacy Call Status (pre-WP execution baseline, 2026-06-05)

| File | Legacy calls | Classification | WP Target |
|---|---|---|---|
| `api/src/esp_hosted_api.c` | 1 (`#if 0` dead code) + 2 old includes | required-active | WP 1 |
| `tools/stats.c` | 2 `esp_hosted_tx()`, 1 `H_DEFLT_FREE_FUNC`, 2 old includes | required-active | WP 1 |
| `serial_ll_if.c` | 2 `esp_hosted_tx()` | required-active | WP 1 (migrate to `h_transmit()`) |
| `vhci_drv.c` | 5 `g_h.funcs->_h_*`, 3 `esp_hosted_tx()`, 3 `H_DEFLT_FREE_FUNC`, 2 old includes | conditional-active | WP 2 |
| `power_save_drv.c` | 31 `g_h.funcs->_h_*`, 2 old includes | required-active | WP 3b |
| `h_osal.c` | timer `H_ERR_NOT_SUP` stub (4 functions) | required-active | WP 3a |
| Legacy port files | `port_esp_hosted_host_os.c` defines `g_h` | legacy-helper | WP 4 |
| SPI/SDIO/SPI-HD/UART leaf drivers | 0 | required-active | Clean (WP from doc 22) |
| All other required-active | 0 | — | Clean |

## 4. Hardware Verification Status

| Transport | Status | Notes |
|---|---|---|
| SPI | `run-capable` | Primary tested path |
| SPI-HD | `run-capable` | Primary tested path |
| SDIO | `unknown` | Hardware availability unconfirmed |
| UART | `unknown` | Hardware availability unconfirmed |

## 5. Adapter Boundary Classification

### Core files (12/12 isolation enforced)
All 12 `host/core/src/*.c` files. Must not include `serial_ll_if.h`, must not reference `serial_ll_if_g->fops`, must not use `g_h.funcs` or `transport_pserial_*`.

### Control adapter files
- `host/port/esp-idf/h_control_serial_adapter.c` — control adapter (moved from core)
- `host/drivers/serial/serial_ll_if.c` — control adapter; `esp_hosted_tx()` to be migrated in WP 1
- `host/drivers/rpc/slaveif/rpc_slave_if.c` — adapter
- `host/drivers/virtual_serial_if/serial_if.c` — adapter

### Transport adapter files (allowed to touch platform APIs)
- `host/port/esp-idf/h_transport_*.c` — new port adapters
- `host/port/esp/freertos/src/port_esp_hosted_host_*.c` — legacy port adapters (WP 4 target)
- `host/drivers/transport/*/` — transport drivers (clean)

### Legacy files (removed or inactive)
- `host/drivers/serial/serial_drv.c` / `serial_drv.h` — deleted (doc 22)
- `host/core/src/h_rpc_slave_if.c` — moved to `host/port/esp-idf/h_control_serial_adapter.c`
- `host/drivers/rpc/core/` — legacy RPC driver, not in CMake active source set
- `host/drivers/transport/transport_drv.c` — legacy transport driver, not in active path

## 6. Control Plane Closure Path

**Selected: Path B** — current-platform adapter closure.

- No new `h_control_contract_t` created
- `h_rpc_slave_if.c` physically moved out of `host/core/src/` to `host/port/esp-idf/`
- `serial_ll_if.c` legacy calls migrated to `h_*` wrappers (except `esp_hosted_tx()` — WP 1 target)
- Core files must not include `serial_ll_if.h` or access `serial_ll_if_g->fops`
- Allowed completion statement: "当前 ESP-IDF 平台控制面 active path 已显式 adapter 化"
- NOT allowed: "控制面 contract 已闭合"

### Upgrade path to Path A
When second-platform PoC restarts, re-evaluate whether to extract `h_control_contract_t`.
The control adapter code will already be isolated in `host/port/esp-idf/`, making extraction straightforward.

## 7. Verification Commands

```bash
# Core isolation (all transports)
scripts/check_core_isolation.sh
scripts/run_linux_mock_tests.sh

# Current-platform isolation (per transport)
scripts/check_current_platform_isolation.sh spi
scripts/check_current_platform_isolation.sh spi_hd
scripts/check_current_platform_isolation.sh sdio
scripts/check_current_platform_isolation.sh uart

# Strict mode (post WP 1-4 — allowlist hit = 0)
scripts/check_current_platform_isolation.sh spi --strict
scripts/check_current_platform_isolation.sh spi_hd --strict
scripts/check_current_platform_isolation.sh sdio --strict
scripts/check_current_platform_isolation.sh uart --strict

# Build verification
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio" build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart" build
```

## 8. Allowlist (post WP 1-6 execution)

### Old-header dependency status: ✅ ALL RESOLVED

All 5 consumer files now use new headers exclusively. The following were migrated:

| File | Legacy headers removed | New headers |
|---|---|---|
| `esp_hosted_api.c` | `port_esp_hosted_host_os.h` + `transport_drv.h` | `h_port_config.h` + `h_transport_drv.h` |
| `vhci_drv.c` | `port_esp_hosted_host_os.h` + `transport_drv.h` | `h_types.h` + `h_transport_drv.h` |
| `power_save_drv.c` | `port_esp_hosted_host_os.h` + `port_esp_hosted_host_config.h` + `transport_drv.h` | `h_port_config.h` + `h_transport_drv.h` |
| `stats.c` | `port_esp_hosted_host_os.h` | `h_types.h` + `h_port_config.h` |
| `serial_ll_if.c` | `port_esp_hosted_host_os.h` + `transport_drv.h` | `h_port_config.h` + `h_transport_drv.h` |

`h_port_config.h` now provides: task defaults, GPIO constants, transport buffer size (per-transport mapping), power-save Kconfig mappings, GPIO reset config.

## 9. Completion Status (2026-06-05, post WP 1-6 execution)

### Post-WP Verification Results

| Check | Result |
|---|---|
| `check_core_isolation.sh` | 12/12 = 100% PASS |
| WP 1: `esp_hosted_api.c` | `#if 0` block removed; `g_h.funcs` zeroed |
| WP 1: `stats.c` | `esp_hosted_tx()` → `h_transmit()`, `H_DEFLT_FREE_FUNC` → `h_free_fn` |
| WP 1: `serial_ll_if.c` | `esp_hosted_tx()` → `h_transmit()` |
| WP 2: `vhci_drv.c` | All `g_h.funcs->_h_*` / `H_DEFLT_FREE_FUNC` → `h_*` wrappers |
| WP 3a: `h_osal.c` timer | Stub → `esp_timer` adapter |
| WP 3b: `power_save_drv.c` | 31 `g_h.funcs->_h_*` → `h_*` wrappers; `h_timer_*` migrated |
| WP 4: `H_DEFLT_FREE_FUNC` | Macro removed from `transport_drv.h` |
| `grep g_h.funcs` across active source set | **0 hits** (legacy port files only) |

### Legacy Call Summary (post-WP)

| File | `g_h.funcs`/`->_h_`/`H_DEFLT_FREE_FUNC`/`esp_hosted_tx` | Old-header status |
|---|---|---|
| `esp_hosted_api.c` | ✅ 0 hits | ✅ Clean (→ `h_port_config.h` + `h_transport_drv.h`) |
| `stats.c` | ✅ 0 hits | ✅ Clean (→ `h_port_config.h` + `h_types.h`) |
| `serial_ll_if.c` | ✅ 0 hits | ✅ Clean (→ `h_port_config.h` + `h_transport_drv.h`) |
| `vhci_drv.c` | ✅ 0 hits | ✅ Clean (→ `h_transport_drv.h` + `h_types.h`) |
| `power_save_drv.c` | ✅ 0 hits | ✅ Clean (→ `h_port_config.h` + `h_transport_drv.h`) |
| `h_osal.c` timer | ✅ Adapters implemented | ✅ Clean |
| Legacy port files | Internally contained | ✅ Legacy helper |
| SPI/SDIO/SPI-HD/UART leaf drivers | ✅ 0 hits | ✅ Clean |

### Allowed Completion Statement (post WP 1-6 execution, 2026-06-05)

> ESP-IDF 当前平台 consumer active path 已完成静态解耦：direct legacy vtable (`g_h.funcs` / `->_h_` / `H_DEFLT_FREE_FUNC`) 与 direct `esp_hosted_tx()` 清零；5 个 consumer 文件旧头 (`transport_drv.h` / `port_esp_hosted_host_os.h` / `port_esp_hosted_host_config.h`) 全部退场，常量统一收口到 `h_port_config.h` / `h_types.h` / `h_transport_drv.h`。Timer contract 已补齐 esp_timer adapter，power-save contract 新增 gpio_pull/gpio_hold 与 HAL slot。ESP-IDF build、`--strict` isolation、pre-commit 待验证。

### NOT Allowed (unchanged)

- "Host 通用框架已经 v1 完成"
- "控制面 contract 已闭合" (Path B = adapter closure, not contract closure)
- "跨平台 contract 充分性已经被真实平台证明"
- "所有 legacy 文件已经删除"
- "ESP32 / ESP-IDF host active source set 已完全解耦" (until WP 1-6 complete)
