# Current Platform Active Path Matrix

> Auto-generated from WP 0 of `22.当前平台完全解耦执行方案.md`
> Date: 2026-06-04
> Control plane closure path: **B (current-platform adapter closure)**

## 1. Active Source Set per Transport

### Always-compiled (all transports)

| Layer | File | Classification |
|---|---|---|
| API | `host/api/src/esp_wifi_weak.c` | adapter |
| API | `host/api/src/esp_hosted_api.c` | migrated to h_* |
| API | `host/api/src/esp_hosted_transport_config.c` | adapter |
| API | `host/api/src/esp_hosted_ota_api.c` | migrated to h_* |
| Core | `host/core/src/h_init.c` | core (12/12 isolation) |
| Core | `host/core/src/h_api.c` | core (12/12 isolation) |
| Core | `host/core/src/h_event.c` | core (12/12 isolation) |
| Core | `host/core/src/h_serial_if.c` | core (12/12 isolation) |
| Core | `host/core/src/h_transport_drv.c` | core (12/12 isolation) |
| Core | `host/core/src/h_transport_util.c` | core (12/12 isolation) |
| Core | `host/core/src/h_rpc_core.c` | core (12/12 isolation) |
| Core | `host/core/src/h_rpc_req.c` | core (12/12 isolation) |
| Core | `host/core/src/h_rpc_rsp.c` | core (12/12 isolation) |
| Core | `host/core/src/h_rpc_evt.c` | core (12/12 isolation) |
| Core | `host/core/src/h_rpc_utils.c` | core (12/12 isolation) |
| Core | `host/core/src/h_rpc_wrap.c` | core (12/12 isolation) |
| RPC driver | `host/drivers/rpc/slaveif/rpc_slave_if.c` | adapter |
| Serial LL | `host/drivers/serial/serial_ll_if.c` | **→ control adapter (WP 1)** |
| Virtual serial | `host/drivers/virtual_serial_if/serial_if.c` | adapter |
| Stats | `host/port/esp-idf/tools/stats.c` | migrated to h_* |
| Power save | `host/drivers/power_save/power_save_drv.c` | legacy adapter (excluded per plan §3.2) |
| CLI | `common/utils/esp_hosted_cli.c` | utility |
| Legacy port | `host/port/esp/freertos/src/port_esp_hosted_host_os.c` | legacy adapter |
| Legacy port | `host/port/esp/freertos/src/port_esp_hosted_host_transport_defaults.c` | legacy adapter |
| New port | `host/port/esp-idf/port_init.c` | adapter |
| New port | `host/port/esp-idf/h_osal.c` | adapter |
| New port | `host/port/esp-idf/h_event.c` | adapter |
| New port | `host/port/esp-idf/h_wifi_type_adapt.c` | adapter |

### Transport-specific

| Transport | Driver | New Port | Legacy Port |
|---|---|---|---|
| SPI | `host/drivers/transport/spi/spi_drv.c` | `host/port/esp-idf/h_transport_spi.c` | `host/port/esp/freertos/src/port_esp_hosted_host_spi.c` |
| SPI-HD | `host/drivers/transport/spi_hd/spi_hd_drv.c` | `host/port/esp-idf/h_transport_spi_hd.c` | `host/port/esp/freertos/src/port_esp_hosted_host_spi_hd.c` |
| SDIO | `host/drivers/transport/sdio/sdio_drv.c` | `host/port/esp-idf/h_transport_sdio.c` | `host/port/esp/freertos/src/port_esp_hosted_host_sdio.c` |
| UART | `host/drivers/transport/uart/uart_drv.c` | `host/port/esp-idf/h_transport_uart.c` | `host/port/esp/freertos/src/port_esp_hosted_host_uart.c` |

## 2. Legacy Call Status

| File | Legacy calls (`g_h.funcs`/`->_h_`) | Status |
|---|---|---|
| `spi_drv.c` | 0 | clean |
| `spi_hd_drv.c` | 0 | clean |
| `sdio_drv.c` | 88 | WP 2 target |
| `uart_drv.c` | 47 | WP 3 target |
| `serial_ll_if.c` | 17 | WP 1 target |
| `serial_drv.c` | 8 | not in active path (inactive file) |

## 3. Hardware Verification Status

| Transport | Status | Notes |
|---|---|---|
| SPI | `run-capable` | Primary tested path |
| SPI-HD | `run-capable` | Primary tested path |
| SDIO | `unknown` | Hardware availability unconfirmed |
| UART | `unknown` | Hardware availability unconfirmed |

## 4. Adapter Boundary Classification

### Core files (12/12 isolation enforced)
All 12 `host/core/src/*.c` files. Must not include `serial_ll_if.h`, must not reference `serial_ll_if_g->fops`, must not use `g_h.funcs` or `transport_pserial_*`.

### Control adapter files (WP 1 completed)
- `host/port/esp-idf/h_control_serial_adapter.c` → control adapter (moved from core; uses `h_control_serial_*` API)
- `host/drivers/serial/serial_ll_if.c` → control adapter; legacy calls migrated to `h_*`
- `host/drivers/rpc/slaveif/rpc_slave_if.c` → adapter
- `host/drivers/virtual_serial_if/serial_if.c` → adapter

### Transport adapter files (allowed to touch platform APIs)
- `host/port/esp-idf/h_transport_*.c` — new port adapters
- `host/port/esp/freertos/src/port_esp_hosted_host_*.c` — legacy port adapters
- `host/drivers/transport/*/` — transport drivers (WP 2/3 target)

### Legacy files (removed or inactive)
- `host/drivers/serial/serial_drv.c` / `serial_drv.h` — **deleted** (replaced by `host/port/esp-idf/h_control_serial_adapter.c`)
- `host/core/src/h_rpc_slave_if.c` — **moved** to `host/port/esp-idf/h_control_serial_adapter.c`
- `host/drivers/rpc/core/` — legacy rpc driver, not in CMake active source set
- `host/drivers/transport/transport_drv.c` — legacy transport driver, not in active path

## 5. Control Plane Closure Path

**Selected: Path B** — current-platform adapter closure.

- No new `h_control_contract_t` created
- `h_rpc_slave_if.c` physically moved out of `host/core/src/` to `host/port/esp-idf/`
- `serial_ll_if.c` legacy calls migrated to `h_*` wrappers
- Core files must not include `serial_ll_if.h` or access `serial_ll_if_g->fops`
- Allowed completion statement: "当前 ESP-IDF 平台控制面 active path 已显式 adapter 化"
- NOT allowed: "控制面 contract 已闭合"

### Upgrade path to Path A
When second-platform PoC restarts, re-evaluate whether to extract `h_control_contract_t`.
The control adapter code will already be isolated in `host/port/esp-idf/`, making extraction straightforward.

## 6. Verification Commands per Transport

```bash
# Core isolation (all transports)
scripts/check_core_isolation.sh
scripts/run_linux_mock_tests.sh

# Current-platform isolation (per transport)
scripts/check_current_platform_isolation.sh spi
scripts/check_current_platform_isolation.sh spi_hd
scripts/check_current_platform_isolation.sh sdio
scripts/check_current_platform_isolation.sh uart

# Build verification
# SPI:    cd examples/host_framework_validation && idf.py set-target esp32p4 && idf.py build
# SPI-HD: (same, with SPI-HD config)
# SDIO:   (same, with SDIO config)
# UART:   (same, with UART config)
```

## 7. Allowlist

| Item | File | Reason | Exit condition |
|---|---|---|---|
| `esp_hosted_tx()` direct call | `serial_ll_if.c` | Control adapter boundary; transmits on ESP_SERIAL_IF | Replace with `h_transmit()` when contract supports non-data-path transmit |
| Legacy `port_esp_hosted_host_os.c` | CMake always-compiled | Provides `g_h.funcs` vtable for legacy files | Remove when no active-path file uses `g_h.funcs` (all drivers now clean) |

## 8. Completion Status (2026-06-04)

### Verification Results

| Check | Result |
|---|---|
| `check_core_isolation.sh` | 12/12 = 100% PASS |
| `check_current_platform_isolation.sh spi` | 9/9 PASS |
| `check_current_platform_isolation.sh spi_hd` | 9/9 PASS |
| `check_current_platform_isolation.sh sdio` | 9/9 PASS |
| `check_current_platform_isolation.sh uart` | 9/9 PASS |

### Legacy Call Migration

| File | Before | After |
|---|---|---|
| `spi_drv.c` | 0 | 0 |
| `spi_hd_drv.c` | 0 | 0 |
| `sdio_drv.c` | 88 | 0 |
| `uart_drv.c` | 47 | 0 |
| `serial_ll_if.c` | 17 | 0 |
| `serial_drv.c` | 8 | DELETED |

### Allowed Completion Statement

> Core (12/12 isolation)、控制面入口 (Path B adapter closure)、四个 transport leaf driver
> (SPI/SPI-HD/SDIO/UART) 与 host API 层均通过显式 contract / wrapper 运行。
> Power save 和 legacy port 实现文件保留为 adapter 边界，不在本轮 closure 范围。

### NOT Allowed

- "Host 通用框架已经 v1 完成"
- "控制面 contract 已闭合" (Path B = adapter closure, not contract closure)
- "跨平台 contract 充分性已经被真实平台证明"
- "所有 legacy 文件已经删除"
