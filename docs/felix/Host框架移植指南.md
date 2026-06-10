# Host 框架移植指南

本文面向准备把 ESP-Hosted-MCU Host 框架移植到新平台的开发者。它只描述当前 contract-based Host 架构下的正确接入方式，不要求实现旧 `g_h` / `hosted_osi_funcs_t` vtable，也不要求复制 `host/port/esp/freertos/`。

如果只想理解架构背景，请先读 `Host框架原理与设计理念.md`。本文是实施手册。

## 1. 移植目标

新增一个 Host 平台时，目标是让现有 core / API / RPC / transport state machine 复用，只替换平台相关能力。

一个新平台至少需要提供：

| 文件 | 作用 |
|---|---|
| `host/port/<port>/h_port_config.h` | 平台配置、transport 选择、feature flags、线程默认值 |
| `host/port/<port>/h_osal.c` | 定义 `g_h_osal`，实现 OSAL contract |
| `host/port/<port>/h_event.c` | 定义 `g_h_event`，实现 event contract |
| `host/port/<port>/h_transport_<bus>.c` | 定义 `g_h_transport`，实现所选 bus 的 transport contract |
| `host/port/<port>/port_init.c` | 定义 `h_port_*` lifecycle hook |
| `host/port/<port>/port.cmake` | 向构建系统导出 port 源文件、include 和依赖 |

可选文件：

| 文件 | 何时需要 |
|---|---|
| `h_transport_<bus>_bus.c` | bus 实现较复杂，建议把 contract adapter 与 bus 操作分开 |
| `h_transport_gpio.c` | 多个 bus 共用 GPIO helper |
| `h_transport_defaults.c` | 多个 bus 共用默认配置 |
| `tools/*.c` | 平台相关调试工具 |

## 2. 不要实现旧接口

新平台不要实现或依赖以下旧接口：

- `hosted_osi_funcs_t`
- `g_h`
- `g_hosted_osi_funcs`
- `esp_hosted_os_abstraction.h`
- `host/port/esp/freertos/*`
- `port_esp_hosted_host_*` 头文件
- `g_h.funcs->_h_*`
- `HOSTED_FREE` / `HOSTED_CALLOC` / `H_DEFLT_FREE_FUNC`

这些属于 legacy compatibility 或未编译旧代码，不是新 port 的参考入口。

正确入口是：

- `host/port/include/h_port_contract.h`
- `host/port/include/h_wrapper.h`
- `host/port/include/h_config.h`
- `host/port/include/h_control_serial_contract.h`

## 3. 推荐目录结构

假设新平台名为 `myrtos`：

```text
host/port/myrtos/
├── port.cmake
├── port_init.c
├── h_port_config.h
├── h_osal.c
├── h_event.c
├── h_transport_spi.c
├── h_transport_spi_bus.c        # optional
├── h_transport_gpio.c           # optional
└── tools/                       # optional
```

如果平台只支持一个 bus，可以只实现一个 `h_transport_<bus>.c`。如果平台同时支持 SPI / SDIO / UART，建议每个 bus 一个 transport adapter，并在 `port.cmake` 中按配置条件选择。

## 4. Port 配置

### 4.1 `h_port_config.h`

`h_port_config.h` 是平台配置入口。它至少需要定义：

```c
#ifndef H_PORT_CONFIG_MYRTOS_H
#define H_PORT_CONFIG_MYRTOS_H

#define H_PORT_NAME         "myrtos"
#define H_PORT_VERSION      "0.1.0"
#define H_PORT_RTOS         "MyRTOS"
#define H_PORT_RTOS_VER     "1.0"
#define H_PORT_CHIP         "my-mcu"
#define H_PORT_BUILD_DATE   __DATE__

/* Exactly one transport must be selected. */
#define H_TRANSPORT_IN_USE  H_TRANSPORT_SPI

/* Thread defaults. */
#define H_DEFAULT_TASK_STACK      4096
#define H_DEFAULT_TASK_PRIO       5
#define H_DEFAULT_RPC_TASK_STACK  H_DEFAULT_TASK_STACK

/* Transport buffer size. */
#define H_MAX_TRANSPORT_BUFFER_SIZE 1600

/* Feature gates. Start small, then enable one by one. */
#define H_FEATURE_BLUETOOTH 0
#define H_FEATURE_OTA       0
#define H_FEATURE_NETSPLIT  0

#endif
```

`H_TRANSPORT_IN_USE` 必须是以下值之一：

| Macro | 含义 |
|---|---|
| `H_TRANSPORT_NONE` | 无 transport，仅用于极小 mock 场景 |
| `H_TRANSPORT_SDIO` | SDIO |
| `H_TRANSPORT_SPI_HD` | SPI half-duplex |
| `H_TRANSPORT_SPI` | SPI full-duplex |
| `H_TRANSPORT_UART` | UART |

### 4.2 配置原则

- 先只启用一个 transport。
- 先关闭 BT、OTA、network split、power-save 等可选功能。
- 先保证 core + control plane + 基础 data path 能编译和初始化。
- 不要为了让代码编译而定义旧 `HOSTED_*` 宏；缺什么就补新 `H_*` 配置或 contract slot。

## 5. 实现 OSAL Contract

OSAL contract 定义在 `host/port/include/h_port_contract.h` 的 `h_osal_contract_t`。

新平台需要在 `h_osal.c` 中定义：

```c
const h_osal_contract_t g_h_osal = {
    .malloc = my_malloc,
    .calloc = my_calloc,
    .realloc = my_realloc,
    .free = my_free,
    .memcpy = my_memcpy,
    .memset = my_memset,
    .malloc_align = my_malloc_align,
    .free_align = my_free_align,

    .thread_create = my_thread_create,
    .thread_delete = my_thread_delete,

    .mutex_create = my_mutex_create,
    .mutex_lock = my_mutex_lock,
    .mutex_unlock = my_mutex_unlock,
    .mutex_delete = my_mutex_delete,

    .queue_create = my_queue_create,
    .queue_send = my_queue_send,
    .queue_recv = my_queue_recv,
    .queue_msg_waiting = my_queue_msg_waiting,
    .queue_reset = my_queue_reset,
    .queue_delete = my_queue_delete,

    .sem_create = my_sem_create,
    .sem_take = my_sem_take,
    .sem_give = my_sem_give,
    .sem_give_from_isr = my_sem_give_from_isr,
    .sem_delete = my_sem_delete,

    .enter_critical = my_enter_critical,
    .exit_critical = my_exit_critical,

    .timer_create = my_timer_create,
    .timer_start = my_timer_start,
    .timer_stop = my_timer_stop,
    .timer_delete = my_timer_delete,
    .get_time_ms = my_get_time_ms,

    .msleep = my_msleep,
    .usleep = my_usleep,
    .blocking_delay = my_blocking_delay,

    .log_write = my_log_write,
};
```

### 5.1 必需能力

| 类别 | 必须实现 |
|---|---|
| Memory | malloc / calloc / realloc / free / memcpy / memset / aligned malloc/free |
| Thread | thread_create / thread_delete |
| Mutex | create / lock / unlock / delete |
| Queue | create / send / recv / msg_waiting / reset / delete |
| Semaphore | create / take / give / give_from_isr / delete |
| Critical | enter / exit |
| Timer | create / start / stop / delete / get_time_ms |
| Delay | msleep / usleep / blocking_delay |
| Logging | log_write |

### 5.2 Timer 语义

当前 contract 约定：

- `timer_create()` 只创建 wrapper handle。
- `timer_start()` 绑定 callback、period 和 one-shot/periodic 语义。
- `timer_stop()` 停止 timer 并释放 handle，是 terminal operation。
- `timer_delete()` 用于 create 后未 start 的错误路径或清理路径。

调用方在 `timer_stop()` 后应将 handle 置空。

如果目标 RTOS 的 timer API 与该语义不完全一致，需要在 OSAL adapter 内完成语义适配，不要把平台 timer 细节泄漏到 core。

### 5.3 ISR 语义

如果平台区分 ISR 与 task context，`sem_give_from_isr()` 必须使用 ISR-safe API。

如果平台没有 ISR context，或者当前 port 的 bring-up 阶段不需要 ISR，可以先映射为普通 `sem_give()`：

```c
static int my_sem_give_from_isr(h_semaphore_t sem, void *isr_ctx)
{
    (void)isr_ctx;
    return my_sem_give(sem);
}
```

### 5.4 Optional OSAL 扩展

以下 slot 可先置空：

| Slot | 场景 |
|---|---|
| `restart_host` | 主机重启 |
| `hosted_init_hook` | port 初始化后 hook |
| `woke_from_ps` | power-save 唤醒检测 |
| `ps_init` | power-save 初始化 |
| `spi_hd_set_data_lines` | SPI-HD 数据线宽切换 |
| `get_host_wakeup_or_reboot_reason` | power-save 唤醒原因 |
| `config_host_power_save_hal` | power-save HAL 配置 |
| `start_host_power_save_hal` | 进入 power-save |

如果功能关闭，optional slot 可以为 `NULL`。wrapper 会通过 `H_VTABLE_CALL` 或显式 guard 返回 `H_ERR_NOT_SUP` / `H_OK`。

## 6. 实现 Event Contract

Event contract 定义在 `h_event_contract_t`。

最小实现：

```c
const h_event_contract_t g_h_event = {
    .register_handler = my_event_register,
    .unregister_handler = my_event_unregister,
    .post = my_event_post,
    .wifi_post = my_event_wifi_post,
};
```

语义要求：

- `register_handler()` 记录 event base、event id、handler、user context。
- `unregister_handler()` 移除匹配 handler。
- `post()` 将事件投递给匹配 handler。
- `wifi_post()` 是 Wi-Fi event 的快捷入口，可直接调用 `post(H_EVENT_WIFI, ...)`。

早期 bring-up 可以用简单链表或固定数组实现 handler registry。优先保证行为正确，后续再优化性能。

## 7. 实现 Transport Contract

Transport contract 定义在 `h_transport_contract_t`。每个 port 只需要填充当前 transport 必需的字段，其余字段可以为 `NULL`。

所有 transport 都必须实现：

| Slot | 说明 |
|---|---|
| `init` | 初始化 bus，输出 bus handle |
| `deinit` | 释放 bus |
| `bus_ready` | 判断 slave bus 是否 ready |
| `transmit` | 发送 Host 到 slave 的 frame |

### 7.1 SPI

当 `H_TRANSPORT_IN_USE == H_TRANSPORT_SPI` 时，必须额外实现：

| Slot | 说明 |
|---|---|
| `spi_transfer` | SPI full-duplex transfer |
| `gpio_config` | 配置 reset/handshake/data-ready 等 GPIO |
| `gpio_set_intr` | 注册 GPIO interrupt |

示例：

```c
const h_transport_contract_t g_h_transport = {
    .init = my_spi_bus_init,
    .deinit = my_spi_bus_deinit,
    .bus_ready = my_spi_bus_ready,
    .transmit = my_spi_transmit,
    .spi_transfer = my_spi_transfer,
    .gpio_config = my_gpio_config,
    .gpio_set_intr = my_gpio_set_intr,
    .gpio_clear_intr = my_gpio_clear_intr,
    .gpio_read = my_gpio_read,
    .gpio_write = my_gpio_write,
};
```

### 7.2 SPI-HD

当 `H_TRANSPORT_IN_USE == H_TRANSPORT_SPI_HD` 时，必须额外实现：

| Slot | 说明 |
|---|---|
| `spi_hd_read_reg` | 读 SPI-HD slave register |
| `spi_hd_write_reg` | 写 SPI-HD slave register |
| `spi_hd_read_dma` | DMA read |
| `spi_hd_write_dma` | DMA write |
| `spi_hd_send_cmd9` | 发送 CMD9 |
| `gpio_config` | GPIO 配置 |
| `gpio_set_intr` | DATA_READY interrupt |

如果平台支持 SPI-HD 1-bit / 2-bit / 4-bit 模式切换，可实现 OSAL optional slot `spi_hd_set_data_lines`。

### 7.3 SDIO

当 `H_TRANSPORT_IN_USE == H_TRANSPORT_SDIO` 时，必须额外实现：

| Slot | 说明 |
|---|---|
| `sdio_card_init` | 初始化 SDIO card |
| `sdio_read_reg` | 读 register |
| `sdio_write_reg` | 写 register |
| `sdio_read_block` | block read |
| `sdio_write_block` | block write |
| `sdio_wait_intr` | 等待 SDIO interrupt |
| `gpio_config` | GPIO 配置 |
| `gpio_write` | reset/wakeup 等 GPIO 输出 |

### 7.4 UART

当 `H_TRANSPORT_IN_USE == H_TRANSPORT_UART` 时，必须额外实现：

| Slot | 说明 |
|---|---|
| `uart_read` | UART read |
| `uart_write` | UART write |
| `uart_flush` | flush input |
| `gpio_config` | reset GPIO 配置 |
| `gpio_write` | reset/wakeup GPIO 输出 |

### 7.5 GPIO 与 netif optional slot

以下 slot 根据功能启用：

| Slot | 何时需要 |
|---|---|
| `gpio_clear_intr` | 需要撤销 interrupt |
| `gpio_read` | 需要读取 GPIO level |
| `gpio_pull` | power-save 或 wakeup pin 需要 pull 配置 |
| `gpio_hold` | deep sleep 期间保持 pin 状态 |
| `netif_create` / `netif_destroy` | port 负责创建网络接口 |

## 8. 控制串口适配

控制面通过 `host/port/include/h_control_serial_contract.h` 暴露。port 需要提供以下函数：

```c
h_control_serial_handle_t *h_control_serial_drv_open(const char *transport);
int h_control_serial_drv_close(h_control_serial_handle_t **handle);
int h_control_serial_drv_write(h_control_serial_handle_t *handle,
                               uint8_t *buf, int in_count, int *out_count);
uint8_t *h_control_serial_drv_read(h_control_serial_handle_t *handle,
                                   uint32_t *out_nbyte);
int h_control_serial_platform_init(void);
int h_control_serial_platform_deinit(void);
```

当前 ESP-IDF port 使用 `serial_ll_if.c` 作为内部 adapter。新平台可以：

- 复用同样的 TLV/control serial 思路。
- 自己实现等价 read/write/open/close。
- 在第二平台 PoC 中再判断是否需要把控制面提升为更正式的独立 contract。

原则：

- core 只 include `h_control_serial_contract.h`。
- port-specific adapter 头不应泄漏到 core。
- 不使用 `transport_pserial_*` 旧接口。

## 9. 实现 Lifecycle Hooks

`port_init.c` 需要定义以下函数：

```c
h_err_t h_port_osal_init(void);
void    h_port_osal_deinit(void);
h_err_t h_port_event_init(void);
void    h_port_event_deinit(void);
h_err_t h_port_transport_init(void);
void    h_port_transport_deinit(void);
h_err_t h_port_rpc_init(void);
void    h_port_rpc_deinit(void);
```

`h_hosted_init()` 的顺序是：

```text
h_validate_contracts()
  -> h_port_osal_init()
  -> h_port_event_init()
  -> h_port_transport_init()
  -> h_port_rpc_init()
```

失败时会按反向顺序回滚已经初始化的层。

推荐 `h_port_rpc_init()` 调用现有 RPC core：

```c
extern int rpc_core_init(void);
extern int rpc_core_start(void);

h_err_t h_port_rpc_init(void)
{
    if (rpc_core_init() != 0)
        return H_FAIL;
    if (rpc_core_start() != 0)
        return H_FAIL;
    return H_OK;
}
```

反初始化：

```c
extern int rpc_core_stop(void);
extern int rpc_core_deinit(void);

void h_port_rpc_deinit(void)
{
    rpc_core_stop();
    rpc_core_deinit();
}
```

## 10. 编写 port.cmake

`port.cmake` 是 port selector 的构建入口。它至少导出：

```cmake
set(ESP_HOSTED_PORT_SRCS
    "${host_dir}/port/myrtos/port_init.c"
    "${host_dir}/port/myrtos/h_osal.c"
    "${host_dir}/port/myrtos/h_event.c"
    "${host_dir}/port/myrtos/h_transport_spi.c"
)

set(ESP_HOSTED_PORT_PRIV_INCLUDE_DIRS
    "${host_dir}/port/myrtos"
    "${host_dir}/port/include"
)

set(ESP_HOSTED_PORT_REQUIRES
)

set(ESP_HOSTED_PORT_PRIV_REQUIRES
)
```

如果支持多个 transport：

```cmake
if(CONFIG_MYRTOS_HOST_SPI)
    list(APPEND ESP_HOSTED_PORT_SRCS
        "${host_dir}/port/myrtos/h_transport_spi.c"
        "${host_dir}/port/myrtos/h_transport_spi_bus.c")
elseif(CONFIG_MYRTOS_HOST_SDIO)
    list(APPEND ESP_HOSTED_PORT_SRCS
        "${host_dir}/port/myrtos/h_transport_sdio.c"
        "${host_dir}/port/myrtos/h_transport_sdio_bus.c")
endif()
```

根 `CMakeLists.txt` 会通过：

```cmake
set(ESP_HOSTED_HOST_PORT "esp-idf" CACHE STRING "ESP-Hosted host port implementation")
include("${host_dir}/port/${ESP_HOSTED_HOST_PORT}/port.cmake")
```

加载 selected port。新增平台时，应让构建系统设置：

```cmake
-DESP_HOSTED_HOST_PORT=myrtos
```

ESP-IDF component build 目前默认仍是 `esp-idf`。非 ESP-IDF 平台可能需要自己的上层构建工程，但 port 目录和 contract 实现仍按本文组织。

## 11. Bring-up 顺序

推荐按以下顺序移植，避免同时调试过多变量：

1. 建立 `host/port/<port>/` 目录和 `port.cmake`。
2. 写最小 `h_port_config.h`，只启用一个 transport，关闭可选功能。
3. 实现 OSAL memory / mutex / semaphore / queue / thread / log。
4. 实现 event registry 的最小版本。
5. 实现 `port_init.c`，让 `h_hosted_init()` 能跑到 contract validation。
6. 实现 transport `init` / `deinit` / `bus_ready`。
7. 打通 slave reset / bus ready / private handshake。
8. 打通 control serial read/write。
9. 跑通最小 RPC，例如获取 MAC、获取版本或 Wi-Fi init。
10. 跑通 STA scan / connect。
11. 再启用 BT、OTA、power-save、network split 等可选功能。

不要一开始就打开全部功能。Host 框架的 feature flags 是为了让移植可以分层推进。

## 12. 验证命令

### 12.1 Core isolation

```bash
bash scripts/check_core_isolation.sh
```

目的：确认 core portable boundary 没有 ESP-IDF / FreeRTOS / legacy vtable 依赖。

### 12.2 当前平台 active path isolation

```bash
bash scripts/check_current_platform_isolation.sh spi --strict
bash scripts/check_current_platform_isolation.sh spi_hd --strict
bash scripts/check_current_platform_isolation.sh sdio --strict
bash scripts/check_current_platform_isolation.sh uart --strict
```

目的：确认 ESP-IDF 当前平台 active path 没有旧 vtable、旧头和控制面边界泄漏。

新平台早期不一定能复用该脚本，但应参考它的检查口径。

### 12.3 Port surface

```bash
bash scripts/check_host_port_surface.sh
```

目的：确认 public/active port surface 没有旧 OS abstraction、旧 port include，且根 CMake 不硬编码 port source。

### 12.4 Linux mock tests

```bash
bash scripts/run_linux_mock_tests.sh
```

目的：验证 core 生产路径在 Linux mock 下可编译、链接和运行。它不验证你的真实硬件 bus。

### 12.5 ESP-IDF host compile check

ESP-IDF 当前平台可用：

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -B build-spi -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" build
```

其他 transport 使用对应 sdkconfig defaults。

注意：不要在仓库根目录直接构建。ESP-Hosted-MCU 是 ESP-IDF component，slave 和每个 example 都是独立 ESP-IDF project。

## 13. 移植完成标准

一个新平台的最小完成标准：

- `h_port_config.h` 定义了平台身份和唯一 transport。
- `g_h_osal` required slot 全部非 NULL。
- `g_h_event` required slot 全部非 NULL。
- `g_h_transport` base slot 和所选 bus 必需 slot 全部非 NULL。
- `h_validate_contracts()` 通过。
- `h_hosted_init()` 成功完成。
- control serial read/write 可用。
- 至少一个最小 RPC request/response 成功。
- 至少一个 Wi-Fi 基本流程成功，例如 scan 或 STA connect。
- 没有新引入 `g_h.funcs`、`HOSTED_*`、`esp_hosted_os_abstraction.h`、`port_esp_hosted_host_*` active 依赖。

推荐完成标准：

- STA connect + DHCP 成功。
- 数据面至少完成一次 TCP/UDP smoke。
- reset/reinit 能重复运行。
- 可选 feature 全部有明确 compile gate。
- CI 至少覆盖 core isolation、mock test、port surface、目标平台 compile。

## 14. 常见错误

### 14.1 复刻旧 vtable

错误：

```c
hosted_osi_funcs_t g_hosted_osi_funcs = { ... };
```

正确：

```c
const h_osal_contract_t g_h_osal = { ... };
const h_event_contract_t g_h_event = { ... };
const h_transport_contract_t g_h_transport = { ... };
```

### 14.2 在 core 里 include port 实现头

错误：

```c
#include "host/port/myrtos/h_transport_spi.h"
```

正确：

```c
#include "h_wrapper.h"
#include "h_port_contract.h"
```

### 14.3 用旧宏绕过 wrapper

错误：

```c
HOSTED_FREE(buf);
g_h.funcs->_h_msleep(100);
```

正确：

```c
h_free(buf);
h_msleep(100);
```

### 14.4 把 optional feature 当 required

错误做法是为了 power-save、BT 或 netif 一开始就实现全部平台能力。

正确做法是先关闭 feature，等主链路跑通后再逐个打开。optional slot 可以先为 `NULL`，但使用路径必须有 feature gate 或 NULL-safe wrapper。

### 14.5 把 `esp_hosted_tx()` 当 public port API

`esp_hosted_tx()` 当前是 transport leaf driver internal API。新平台优先实现 `g_h_transport.transmit`。不要让 core、API、tools、BT、power-save 直接调用 `esp_hosted_tx()`。

## 15. 推荐阅读代码

| 目的 | 文件 |
|---|---|
| Contract 定义 | `host/port/include/h_port_contract.h` |
| Wrapper 调用方式 | `host/port/include/h_wrapper.h` |
| 通用配置入口 | `host/port/include/h_config.h` |
| 控制串口 contract | `host/port/include/h_control_serial_contract.h` |
| 初始化和校验 | `host/core/src/h_init.c` |
| ESP-IDF OSAL 参考 | `host/port/esp-idf/h_osal.c` |
| ESP-IDF event 参考 | `host/port/esp-idf/h_event.c` |
| ESP-IDF port source list | `host/port/esp-idf/port.cmake` |
| Linux mock 参考 | `host/port/linux/` |

## 16. 一句话总结

移植新平台时，不要从旧 ESP/Freertos port 或 `esp_hosted_os_abstraction.h` 开始。正确路径是新增 `host/port/<platform>/`，实现 `g_h_osal`、`g_h_event`、`g_h_transport` 和 `port.cmake`，先关闭可选功能跑通最小 transport + control RPC，再逐步打开完整 Host 功能。
