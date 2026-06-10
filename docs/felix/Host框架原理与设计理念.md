# Host 框架原理与设计理念

本文描述 ESP-Hosted-MCU Host 框架在第一阶段重构完成后的长期架构口径。它不记录历史执行过程，也不作为某次重构的任务清单；它只回答三个问题：

1. Host 框架为什么要分层。
2. 当前代码如何通过 contract/wrapper 与 port selector 隔离平台。
3. 第一阶段结束后，哪些结论可以成立，哪些结论仍不能过度声明。

## 1. 当前结论

截至当前代码状态，ESP-IDF Host active source set 已完成 legacy port/vtable 解耦：

- `host/port/esp/freertos/src/*.c` 不再进入 ESP-IDF Host active source set。
- `host/port/esp/freertos/include` 不再进入 ESP-IDF Host include path。
- active path 中不再依赖旧 `g_h.funcs` vtable、`->_h_` 调用、`HOSTED_FREE` / `HOSTED_CALLOC` 宏或 `H_DEFLT_FREE_FUNC` 宏链。
- core 层不直接 include ESP-IDF / FreeRTOS / legacy port 头。
- Host port 入口已由根 `CMakeLists.txt` 收口为 `ESP_HOSTED_HOST_PORT` + `host/port/<port>/port.cmake`。
- 新平台移植入口已明确为实现 `h_port_contract.h` 中的 contract，而不是复刻旧 `hosted_osi_funcs_t`。

允许的表述：

> 当前 ESP-IDF Host active source set 已完成 legacy port/vtable 解耦；core 层保持平台隔离；port 接入模型已收口为三合约 + wrapper + port.cmake selector。该状态达到当前平台移植友好型架构要求。

不允许的表述：

- 任意第二平台已经完成验证。
- 所有 legacy 文件已经删除。
- 所有 active driver 源码都已经变成平台无关 C。
- `esp_hosted_tx()` 已升级为 public port contract。

更精确地说：当前完成的是当前 ESP-IDF 平台 active path 解耦与 porting architecture closure；第二平台 PoC 是后续证明 contract 充分性的独立阶段。

## 2. 设计目标

Host 框架的目标不是把所有代码都写成完全平台无关，而是把平台差异限制在可识别、可替换、可验证的边界内。

核心目标：

- core 层负责协议、RPC、状态机和业务编排。
- port 层负责 OS、event、transport、GPIO、timer、logging 等平台能力。
- driver 层可以包含当前平台实现细节，但不能绕回旧 vtable 或隐式 port 入口。
- 新平台移植者只需要面对明确 contract，而不是在旧 ESP-IDF port、旧 FreeRTOS helper、旧 vtable 和 driver 内部 API 之间猜边界。

非目标：

- 不在第一阶段删除所有旧文件。
- 不改变 wire protocol。
- 不改变 public Wi-Fi / BT / OTA API 行为。
- 不把第二平台 PoC 结果提前写成事实。
- 不强行把 transport leaf driver 内部 API 全部升级成 public contract。

## 3. 分层模型

当前 Host 框架按以下层次组织：

```text
Application / ESP-IDF examples
        |
Public API
        |
Core layer
  host/core/src
  - RPC request/response/event
  - transport state machine
  - Wi-Fi API wrapper logic
  - no direct ESP-IDF / FreeRTOS / legacy port dependency
        |
Wrapper layer
  host/port/include/h_wrapper.h
  - h_malloc, h_sem_take, h_transmit, H_LOGE, ...
        |
Contract layer
  host/port/include/h_port_contract.h
  - g_h_osal
  - g_h_event
  - g_h_transport
        |
Selected port
  host/port/<port>/
  - h_osal.c
  - h_event.c
  - h_transport_*.c
  - port_init.c
  - port.cmake
        |
Platform SDK / RTOS / bus driver
```

这个结构的关键点是单向依赖：

- core 依赖 wrapper。
- wrapper 依赖 contract。
- contract 只定义能力，不依赖具体平台。
- port 实现 contract。
- 根构建系统只选择一个 port。

任何平台能力都应从 core 通过 `h_*` wrapper 进入 port，不应从 core 直接 include 平台头或调用平台 API。

## 4. Core 层原则

core 层是 Host 框架中最重要的可移植边界。它的职责包括：

- Host 初始化与反初始化编排。
- RPC encode/decode、request/response 匹配、event dispatch。
- Wi-Fi API 到 RPC 的桥接。
- transport 状态机和 channel 管理。
- 与平台无关的错误码、类型和控制逻辑。

core 层不应该做的事情：

- include `esp_log.h`、`sdkconfig.h`、`freertos/*`、`esp_heap_caps.h` 等平台头。
- 调用 `ESP_LOG*`、FreeRTOS API、ESP-IDF API。
- 访问 `g_h.funcs`、`->_h_` 或 `HOSTED_*` legacy macro。
- include `esp_hosted_os_abstraction.h`。
- include `host/port/esp-idf/*` 下的具体实现头。

core 层需要平台能力时，只能使用：

- `h_wrapper.h` 中的 wrapper。
- `h_config.h` 中的通用配置入口。
- `h_port_contract.h` 中定义的 contract 类型。
- `h_control_serial_contract.h` 中定义的控制串口 contract。

## 5. Contract 与 Wrapper

### 5.1 为什么使用 contract

Host 框架原先依赖 `hosted_osi_funcs_t` / `g_h.funcs` 这类大而全的 legacy vtable。它的问题是：

- OS、event、transport、GPIO、power-save 等能力混在一个结构中。
- 旧 vtable 名称和语义绑定历史实现，新平台难以判断哪些必须实现。
- active path 与 legacy helper 容易互相引用，导致“看似抽象，实际仍依赖旧 port”。

第一阶段后的新结构把平台能力拆成三个主要 contract：

| Contract | 全局实例 | 职责 |
|---|---|---|
| `h_osal_contract_t` | `g_h_osal` | memory、thread、mutex、queue、semaphore、timer、logging、部分 power-save optional extension |
| `h_event_contract_t` | `g_h_event` | event register、unregister、post、Wi-Fi event post |
| `h_transport_contract_t` | `g_h_transport` | transport lifecycle、transmit、bus-specific operation、GPIO、netif |

控制串口另有一个轻量 contract：

| Contract | 位置 | 职责 |
|---|---|---|
| `h_control_serial_contract.h` | `host/port/include` | 控制面 serial open/read/write/close/platform init/deinit 声明 |

### 5.2 为什么使用 wrapper

core 不直接写 `g_h_osal.malloc(...)`，而是写 `h_malloc(...)`。这一层 wrapper 有三个作用：

- 让 core 代码表达业务意图，而不是暴露 vtable 字段。
- 允许 contract 字段替换而不大规模修改 core。
- 对 optional slot 统一做 NULL guard，例如 `H_VTABLE_CALL(...)`。

示例：

```c
buf = h_calloc(1, len);
h_sem_take(sem, H_BLOCK_MAX);
ret = h_transmit(if_type, if_num, payload, len, zcopy, to_free, h_free_fn, flags);
H_LOGE(TAG, "failed: %d", ret);
```

这些调用最终由当前 selected port 的 `g_h_osal`、`g_h_event`、`g_h_transport` 实现。

### 5.3 Required 与 optional

contract 中不是所有字段都对所有平台必需。规则是：

- OSAL 基础能力是 required。
- Event 基础能力是 required。
- Transport base 能力是 required。
- Bus-specific 能力只在对应 `H_TRANSPORT_IN_USE` 下 required。
- Power-save、GPIO hold/pull、netif 等能力可作为 optional extension。

初始化时 `h_validate_contracts()` 会根据当前 transport 做 fail-fast 校验。如果 required slot 缺失，`h_hosted_init()` 直接失败。

## 6. Port Selector

根 `CMakeLists.txt` 不再硬编码 ESP-IDF port 的全部源文件，而是通过 selected port 的 `port.cmake` 注入：

```cmake
set(ESP_HOSTED_HOST_PORT "esp-idf" CACHE STRING "ESP-Hosted host port implementation")
include("${host_dir}/port/${ESP_HOSTED_HOST_PORT}/port.cmake")

list(APPEND srcs ${ESP_HOSTED_PORT_SRCS})
list(APPEND priv_include ${ESP_HOSTED_PORT_PRIV_INCLUDE_DIRS})
list(APPEND driver_requires ${ESP_HOSTED_PORT_REQUIRES})
```

这个机制的意义是：

- 当前默认 port 仍是 `esp-idf`。
- 新 port 的工程入口是 `host/port/<new-port>/port.cmake`。
- 根 CMake 不需要知道每个 port 的具体源文件。
- port 的源文件、私有 include、public requires、private requires 由 port 自己导出。

这不是第二平台已经可直接构建的证明，但它把第二平台接入点从“修改根 CMake 并猜 active path”降为“新增一个 port 目录并导出 port.cmake”。

## 7. ESP-IDF Port 的定位

`host/port/esp-idf/` 是当前生产参考 port。它负责：

- 将 `h_osal_contract_t` 映射到 FreeRTOS、ESP-IDF timer、heap、log 等能力。
- 将 `h_event_contract_t` 映射到 ESP-IDF event loop。
- 将 `h_transport_contract_t` 映射到 SPI、SPI-HD、SDIO、UART bus helper。
- 提供 `h_port_config.h`，把 Kconfig 选项映射为 `H_*` 配置。
- 提供 `port.cmake`，把 ESP-IDF port 源文件注入组件构建。

ESP-IDF port 可以 include ESP-IDF / FreeRTOS 头，也可以使用平台原生 API。它是平台边界，不是 portable boundary。

## 8. Transport 边界

Transport 分两层：

```text
core transport state machine
        |
h_transport_contract_t / h_wrapper.h
        |
host/port/esp-idf/h_transport_<bus>.c
        |
host/port/esp-idf/h_transport_<bus>_bus.c
        |
host/drivers/transport/<bus>/<bus>_drv.c
```

当前 `esp_hosted_tx()` 仍存在于 transport leaf driver 中，并由 ESP-IDF port adapter 的 `transmit` 槽封装调用。它的定位是 transport leaf driver internal API，不是 public port contract。

因此正确理解是：

- consumer 侧不应直接调用 `esp_hosted_tx()`。
- core、API、tools、BT、power-save 等 consumer 应通过 `h_transmit()`。
- transport leaf driver 内部可以定义或调用 `esp_hosted_tx()`。
- 如果未来要支持更多非 ESP-IDF transport driver，可再评估是否把 leaf driver internal API 进一步抽象。

## 9. Control Plane 边界

控制面当前采用 adapter closure，而不是单独抽象成完整 `h_control_contract_t`：

- core 侧只依赖 `h_control_serial_contract.h`。
- ESP-IDF 实现位于 `host/port/esp-idf/h_control_serial_adapter.c`。
- `h_control_serial_adapter.h` 只作为兼容 redirect，实际声明收口在 `host/port/include/h_control_serial_contract.h`。
- `serial_ll_if.c` 是内部 serial adapter，已脱离旧 `g_h.funcs` 路径。

这意味着当前控制面在 active path 上已经脱离 legacy vtable，但不声明“控制面独立 contract 已作为跨平台公共规范完全定型”。第二平台 PoC 开始时，可以重新评估是否需要把控制面升级为独立 contract。

## 10. Legacy 兼容边界

以下文件或概念仍可能存在于仓库中，但不应作为新架构入口：

- `host/esp_hosted_os_abstraction.h`
- `hosted_osi_funcs_t`
- `g_h`
- `g_hosted_osi_funcs`
- `host/port/esp/freertos/`
- 旧 RPC / transport duplicate source 中的 legacy 调用

这些保留的意义是兼容和历史参考，不代表 active path 仍依赖它们。新代码和新 port 不应 include 或实现这些 legacy 入口。

对外判断时要区分：

| 范围 | 结论 |
|---|---|
| ESP-IDF active source set | 已完成 legacy port/vtable 解耦 |
| core portable boundary | 已保持平台隔离 |
| 全仓所有 legacy 文件 | 未删除，不作为完成标准 |
| 第二平台真实 PoC | 未完成，不作为当前声明 |

## 11. 验证体系

当前架构通过以下层次验证：

| 验证 | 目的 |
|---|---|
| `scripts/check_core_isolation.sh` | 检查 core portable boundary 是否出现 ESP-IDF / FreeRTOS / legacy vtable 依赖 |
| `scripts/check_current_platform_isolation.sh <transport> --strict` | 检查当前平台 active path 是否出现旧 vtable、旧头、控制面边界泄漏 |
| `scripts/check_host_port_surface.sh` | 检查 public/active port surface 是否泄漏旧 OS abstraction、旧 port include、根 CMake 硬编码 port 源 |
| `scripts/run_linux_mock_tests.sh` | Linux mock 编译、链接、ASAN/UBSAN/TSAN 测试 |
| ESP-IDF host build matrix | 验证当前 ESP-IDF 平台在各 transport 下可构建 |

这些验证能证明当前平台 active path 的静态解耦和参考 port 的构建可用性。它们不能替代第二平台真实 bring-up。

## 12. 当前阶段的最佳实践判断

当前 Host 框架已达到当前平台“移植友好型最佳实践”的架构要求，原因是：

- core 层没有直接平台依赖。
- 平台能力通过明确 contract 暴露。
- wrapper 统一了调用入口。
- ESP-IDF port 不再与 legacy FreeRTOS port 双层并行编译。
- `port.cmake` 建立了新 port 的构建入口。
- porting guide 可直接指导新平台实现三合约。

但仍需保留两个边界判断：

1. active driver 源码中仍可能使用 ESP-IDF 日志或 FreeRTOS 头。这属于当前 ESP-IDF driver 实现层，不等价于 core 耦合。
2. 第二平台尚未完成真实 PoC，因此不能宣称 contract 对任意平台已经充分。

## 13. 后续演进方向

第一阶段结束后，后续工作建议按以下优先级推进：

1. 用第二平台 PoC 验证 contract 充分性。
2. 根据 PoC 结果决定是否拆出更细的 control、GPIO、power-save 或 bus contract。
3. 清理或归档未编译 legacy duplicate source，降低全仓审计噪音。
4. 逐步减少 active driver 对 ESP-IDF 日志宏的直接使用，但不要把它与 core 解耦完成度混为一谈。
5. 将文档长期维护为两份：本设计理念文档和 Host 框架移植指南。

## 14. 一句话总结

Host 框架第一阶段重构已经把平台差异从旧 `g_h.funcs` 大 vtable 和 legacy port 双层结构中释放出来，收口为清晰的 contract/wrapper/port selector 模型。当前 ESP-IDF 平台 active path 已完成解耦；下一阶段的核心任务不是继续证明 ESP-IDF 自己，而是用第二平台验证这些 contract 是否足够通用。
