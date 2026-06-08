# Legacy Port 收口退场 — Implementation Plan

**Goal:** Remove `host/port/esp/freertos/` from the ESP-IDF host build by migrating its contents into `host/port/esp-idf/` with clean naming and explicit internal driver APIs.

**Architecture:** Two-phase migration. First, move GPIO/bus/config helpers into new `host/port/esp-idf/` files using either renamed functions (GPIO/bus → no symbol conflict) or atomic CMake swap (default config → same names, must swap in one commit). Second, replace legacy `#include` paths in active source files, then remove `host/port/esp/freertos/` from CMakeLists entirely.

**Tech Stack:** C (ESP-IDF component), CMake, bash verification scripts

---

### Task 1: Create infrastructure files (WP 1)

**Files:**
- Create: `host/drivers/transport/transport_drv_api.h`
- Create: `host/port/esp-idf/h_transport_gpio.h`
- Create: `host/port/esp-idf/h_transport_gpio.c`
- Create: `host/port/esp-idf/h_transport_spi_bus.h`
- Create: `host/port/esp-idf/h_transport_spi_bus.c`
- Create: `host/port/esp-idf/h_transport_spi_hd_bus.h`
- Create: `host/port/esp-idf/h_transport_spi_hd_bus.c`
- Create: `host/port/esp-idf/h_transport_sdio_bus.h`
- Create: `host/port/esp-idf/h_transport_sdio_bus.c`
- Create: `host/port/esp-idf/h_transport_uart_bus.h`
- Create: `host/port/esp-idf/h_transport_uart_bus.c`
- Create: `host/port/esp-idf/h_transport_defaults.c`
- Create: `host/port/esp-idf/h_transport_common.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Create `transport_drv_api.h`**

```c
/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TRANSPORT_DRV_API_H
#define TRANSPORT_DRV_API_H

/* Internal driver API — not a porting contract.
 * Each transport leaf driver (spi / sdio / spi_hd / uart) must provide
 * these symbols. Upper layers (core transport, port adapters) call them
 * via this header instead of bare extern declarations. */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  esp_hosted_tx(uint8_t iface_type, uint8_t iface_num,
                   uint8_t *payload_buf, uint16_t payload_len,
                   uint8_t buff_zerocopy, uint8_t *buffer_to_free,
                   void (*free_buf_func)(void *ptr), uint8_t flags);

void check_if_max_freq_used(uint8_t chip_type);

int  ensure_slave_bus_ready(void *bus_handle);

#ifdef __cplusplus
}
#endif

#endif /* TRANSPORT_DRV_API_H */
```

- [ ] **Step 2: Create empty bus/GPIO/common `.c` and `.h` files**

Create each file with only the copyright header and an include guard (for `.h` files). Example for `h_transport_gpio.h`:

```c
/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_GPIO_H
#define H_TRANSPORT_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO helpers — ESP-IDF port internal API */

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_GPIO_H */
```

Create all these files (each `.h` gets a matching include guard with the appropriate name):
- `host/port/esp-idf/h_transport_gpio.h` / `.c`
- `host/port/esp-idf/h_transport_spi_bus.h` / `.c`
- `host/port/esp-idf/h_transport_spi_hd_bus.h` / `.c`
- `host/port/esp-idf/h_transport_sdio_bus.h` / `.c`
- `host/port/esp-idf/h_transport_uart_bus.h` / `.c`
- `host/port/esp-idf/h_transport_defaults.c`
- `host/port/esp-idf/h_transport_common.c`

For each `.h` file, use an include guard like `H_TRANSPORT_SPI_BUS_H`, `H_TRANSPORT_SDIO_BUS_H`, etc.

- [ ] **Step 3: Add new files to CMakeLists**

In `CMakeLists.txt`, after the line adding `"${host_dir}/port/esp-idf/h_control_serial_adapter.c"`, add:

```cmake
		"${host_dir}/port/esp-idf/h_transport_gpio.c"
		"${host_dir}/port/esp-idf/h_transport_defaults.c"
		"${host_dir}/port/esp-idf/h_transport_common.c"
```

And inside the per-transport conditional blocks, add the bus files. For SPI (after `h_transport_spi.c`):

```cmake
			list(APPEND srcs "${host_dir}/port/esp-idf/h_transport_spi_bus.c")
```

Do the same for `spi_hd`, `sdio`, and `uart` blocks.

- [ ] **Step 4: Verify empty files compile**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: Compiles successfully (no new symbols, no conflicts).

- [ ] **Step 5: Commit**

```bash
git add host/drivers/transport/transport_drv_api.h \
        host/port/esp-idf/h_transport_gpio.h host/port/esp-idf/h_transport_gpio.c \
        host/port/esp-idf/h_transport_spi_bus.h host/port/esp-idf/h_transport_spi_bus.c \
        host/port/esp-idf/h_transport_spi_hd_bus.h host/port/esp-idf/h_transport_spi_hd_bus.c \
        host/port/esp-idf/h_transport_sdio_bus.h host/port/esp-idf/h_transport_sdio_bus.c \
        host/port/esp-idf/h_transport_uart_bus.h host/port/esp-idf/h_transport_uart_bus.c \
        host/port/esp-idf/h_transport_defaults.c host/port/esp-idf/h_transport_common.c \
        CMakeLists.txt
git commit -m "feat: add infrastructure files for legacy port migration"
```

---

### Task 2: GPIO helper migration with new naming (WP 2)

**Files:**
- Modify: `host/port/esp-idf/h_transport_gpio.h`
- Modify: `host/port/esp-idf/h_transport_gpio.c`
- Modify: `host/port/esp-idf/h_transport_spi.c`
- Modify: `host/port/esp-idf/h_transport_sdio.c`
- Modify: `host/port/esp-idf/h_transport_spi_hd.c`
- Modify: `host/port/esp-idf/h_transport_uart.c`

- [ ] **Step 1: Write GPIO header with new function declarations**

Replace the content of `host/port/esp-idf/h_transport_gpio.h`:

```c
/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_GPIO_H
#define H_TRANSPORT_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO helpers — ESP-IDF port internal API.
 * These replace the legacy hosted_config_gpio() / hosted_*_gpio()
 * functions that were defined in port_esp_hosted_host_os.c. */

int h_gpio_config(uint32_t gpio_num, uint32_t mode);
int h_gpio_setup_intr(uint32_t gpio_num, uint32_t intr_type,
                      void (*fn)(void *), void *arg);
int h_gpio_clear_intr(uint32_t gpio_num);
int h_gpio_read(uint32_t gpio_num);
int h_gpio_write(uint32_t gpio_num, uint32_t value);
int h_gpio_pull(uint32_t gpio_num, uint32_t pull_value, uint32_t enable);
int h_gpio_hold(uint32_t gpio_num, uint32_t hold_value);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_GPIO_H */
```

- [ ] **Step 2: Write GPIO implementation**

Replace the content of `host/port/esp-idf/h_transport_gpio.c`:

```c
/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "h_transport_gpio.h"
#include "h_wrapper.h"
#include "h_port_config.h"

#include <driver/gpio.h>
#include <esp_log.h>

#define TAG "h_gpio"

int h_gpio_config(uint32_t gpio_num, uint32_t mode)
{
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = mode,
        .pin_bit_mask = (1ULL << gpio_num),
        .pull_down_en = 0,
        .pull_up_en   = 0,
    };
    ESP_LOGI(TAG, "GPIO [%d] configured", (int)gpio_num);
    gpio_config(&io_conf);
    return 0;
}

int h_gpio_setup_intr(uint32_t gpio_num, uint32_t intr_type,
                      void (*fn)(void *), void *arg)
{
    static bool isr_service_installed = false;

    gpio_config_t new_gpio_io_conf = {
        .mode         = GPIO_MODE_INPUT,
        .intr_type    = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << gpio_num),
    };

    if (intr_type == H_GPIO_INTR_NEGEDGE) {
        new_gpio_io_conf.pull_up_en = 1;
    } else {
        new_gpio_io_conf.pull_down_en = 1;
    }

    ESP_LOGI(TAG, "GPIO [%d] configuring as Interrupt", (int)gpio_num);
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&new_gpio_io_conf));

    if (!isr_service_installed) {
        gpio_install_isr_service(0);
        isr_service_installed = true;
    }

    gpio_isr_handler_remove(gpio_num);
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_isr_handler_add(gpio_num, fn, arg));

    int ret = gpio_set_intr_type(gpio_num, intr_type);
    if (ret != ESP_OK) {
        gpio_isr_handler_remove(gpio_num);
        return ret;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_intr_enable(gpio_num));
    return ret;
}

int h_gpio_clear_intr(uint32_t gpio_num)
{
    gpio_intr_disable(gpio_num);
    return gpio_isr_handler_remove(gpio_num);
}

int h_gpio_read(uint32_t gpio_num)
{
    return gpio_get_level(gpio_num);
}

int h_gpio_write(uint32_t gpio_num, uint32_t value)
{
    return gpio_set_level(gpio_num, value);
}

int h_gpio_pull(uint32_t gpio_num, uint32_t pull_value, uint32_t enable)
{
    if (pull_value == H_GPIO_PULL_UP) {
        return enable ? gpio_pullup_en(gpio_num) : gpio_pullup_dis(gpio_num);
    } else {
        return enable ? gpio_pulldown_en(gpio_num) : gpio_pulldown_dis(gpio_num);
    }
}

int h_gpio_hold(uint32_t gpio_num, uint32_t hold_value)
{
    if (hold_value) {
        return gpio_hold_en(gpio_num);
    } else {
        return gpio_hold_dis(gpio_num);
    }
}
```

- [ ] **Step 3: Update `h_transport_spi.c` — replace extern GPIO + use new names**

In `host/port/esp-idf/h_transport_spi.c`:
- Delete the GPIO `extern hosted_*gpio*` declarations
- Add `#include "h_transport_gpio.h"` after the existing `#include <driver/gpio.h>`
- Replace `hosted_config_gpio(NULL, pin, mode)` with `h_gpio_config(pin, mode)`
- Replace `hosted_setup_gpio_interrupt(NULL, pin, intr_type, isr, arg)` with `h_gpio_setup_intr(pin, intr_type, isr, arg)`
- Replace `hosted_teardown_gpio_interrupt(NULL, pin)` with `h_gpio_clear_intr(pin)`
- Replace `hosted_read_gpio(NULL, pin)` with `h_gpio_read(pin)`
- Replace `hosted_write_gpio(NULL, pin, value)` with `h_gpio_write(pin, value)`
- Keep the `extern hosted_spi_init()` etc. and `extern esp_hosted_tx()` for now (Task 6 will replace them)

- [ ] **Step 4: Update `h_transport_sdio.c` — same pattern**

Same changes: delete GPIO externs, add `#include "h_transport_gpio.h"`, replace all `hosted_*gpio*(NULL, pin, ...)` calls with `h_gpio_*(pin, ...)` calls. Keep SDIO bus externs for now.

- [ ] **Step 5: Update `h_transport_spi_hd.c` — same pattern**

Same changes. Keep SPI-HD bus externs for now.

- [ ] **Step 6: Update `h_transport_uart.c` — same pattern**

Same changes. Keep UART bus externs for now.

- [ ] **Step 7: Build verify SPI transport**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: compile success.

- [ ] **Step 8: Commit**

```bash
git add host/port/esp-idf/h_transport_gpio.h host/port/esp-idf/h_transport_gpio.c \
        host/port/esp-idf/h_transport_spi.c host/port/esp-idf/h_transport_sdio.c \
        host/port/esp-idf/h_transport_spi_hd.c host/port/esp-idf/h_transport_uart.c
git commit -m "feat: migrate GPIO helpers to h_transport_gpio.c with new naming"
```

---

### Task 3: Default transport config migration — atomic CMake swap (WP 3a)

**Files:**
- Modify: `host/port/esp-idf/h_transport_defaults.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write `h_transport_defaults.c` with the 4 config functions**

Replace the content of `host/port/esp-idf/h_transport_defaults.c` with the complete implementation. Copy the 4 `esp_hosted_get_default_*_config()` functions plus `esp_hosted_get_default_sdio_iomux_config()` from `host/port/esp/freertos/src/port_esp_hosted_host_transport_defaults.c`. Change `#include "port_esp_hosted_host_config.h"` to `#include "h_port_config.h"`. Change `#ifdef` to `#if`. Preserve all function names exactly.

- [ ] **Step 2: Atomic CMakeLists swap**

In `CMakeLists.txt`, in the same edit:
- Remove `"${host_dir}/port/esp/freertos/src/port_esp_hosted_host_transport_defaults.c"` from the legacy port srcs list
- Confirm `h_transport_defaults.c` is already in the srcs list (added in Task 1 Step 3)

The above two changes MUST be in the same commit.

- [ ] **Step 3: Build verify all four transports**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi"    -B build-spi    build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" -B build-spi_hd build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio"   -B build-sdio   build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart"   -B build-uart   build
```

Expected: All four compile. No duplicate symbol errors.

- [ ] **Step 4: Commit**

```bash
git add host/port/esp-idf/h_transport_defaults.c CMakeLists.txt
git commit -m "feat: migrate default transport config to h_transport_defaults.c (atomic CMake swap)"
```

---

### Task 4: Wi-Fi feature flag header migration (WP 3b)

**Files:**
- Modify: `host/port/esp-idf/h_port_config.h`
- Modify: `host/api/src/esp_wifi_weak.c`

- [ ] **Step 1: Add Wi-Fi feature flags to `h_port_config.h`**

In `host/port/esp-idf/h_port_config.h`, append before the final `#endif`. Skip `H_WIFI_NEW_RESERVED_FIELD_NAMES`, `H_PRESENT_IN_ESP_IDF_5_4_0`, `H_PRESENT_IN_ESP_IDF_5_5_0`, `H_DECODE_WIFI_RESERVED_FIELD` — they already exist. Add the remaining macros from `port_esp_hosted_host_wifi_config.h`:

```c
/* ── Wi-Fi feature flags (from legacy port_esp_hosted_host_wifi_config.h) ── */

#if CONFIG_ESP_HOSTED_ENABLE_ITWT && CONFIG_SLAVE_SOC_WIFI_HE_SUPPORT
  #define H_WIFI_HE_SUPPORT 1
#else
  #define H_WIFI_HE_SUPPORT 0
#endif

#if H_WIFI_HE_SUPPORT && (ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 3, 0))
  #define H_WIFI_HE_GREATER_THAN_ESP_IDF_5_3 1
#else
  #define H_WIFI_HE_GREATER_THAN_ESP_IDF_5_3 0
#endif

#ifndef H_WIFI_DUALBAND_SUPPORT
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
  #define H_WIFI_DUALBAND_SUPPORT 1
#else
  #define H_WIFI_DUALBAND_SUPPORT 0
#endif
#endif

#ifdef CONFIG_ESP_WIFI_REMOTE_EAP_ENABLED
  #define H_WIFI_ENTERPRISE_SUPPORT 1
#else
  #define H_WIFI_ENTERPRISE_SUPPORT 0
#endif

#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 3, 1)
  #define H_GOT_TWT_ENABLE_KEEP_ALIVE 1
#else
  #define H_GOT_TWT_ENABLE_KEEP_ALIVE 0
#endif

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 3) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)) || \
    (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 1))
  #define H_GOT_AP_CONFIG_PARAM_TRANSITION_DISABLE 1
#else
  #define H_GOT_AP_CONFIG_PARAM_TRANSITION_DISABLE 0
#endif

#ifndef H_PRESENT_IN_ESP_IDF_6_0_0
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
  #define H_PRESENT_IN_ESP_IDF_6_0_0  1
#else
  #define H_PRESENT_IN_ESP_IDF_6_0_0  0
#endif
#endif

#if ((ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 4) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)) || \
     (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 3) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 5, 0)) || \
     (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 1)))
  #define H_GOT_SET_EAP_METHODS_API 1
#else
  #define H_GOT_SET_EAP_METHODS_API 0
#endif

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 4) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)) || \
    (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 3))
  #define H_GOT_EAP_SET_DOMAIN_NAME 1
#else
  #define H_GOT_EAP_SET_DOMAIN_NAME 0
#endif

#if (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 3))
  #define H_GOT_EAP_OKC_SUPPORT 0
#else
  #define H_GOT_EAP_OKC_SUPPORT 1
#endif

/* DPP (Wi-Fi Easy Connect) support */
#if CONFIG_ESP_HOSTED_ENABLE_DPP && (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0))
  #define H_SUPP_DPP_SUPPORT 1
#else
  #define H_SUPP_DPP_SUPPORT 0
#endif

#if CONFIG_ESP_HOSTED_ENABLE_DPP && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0))
  #define H_WIFI_DPP_SUPPORT 1
#else
  #define H_WIFI_DPP_SUPPORT 0
#endif

#if H_SUPP_DPP_SUPPORT || H_WIFI_DPP_SUPPORT
  #define H_DPP_SUPPORT 1
#else
  #define H_DPP_SUPPORT 0
#endif

#if H_DPP_SUPPORT
  #define H_DPP_URI_LEN_MAX CONFIG_ESP_HOSTED_DPP_URI_LEN_MAX
#endif
```

- [ ] **Step 2: Update `esp_wifi_weak.c` includes**

In `host/api/src/esp_wifi_weak.c`, change:
```c
#include "port_esp_hosted_host_config.h"
#include "port_esp_hosted_host_wifi_config.h"
```
to:
```c
#include "h_port_config.h"
```

- [ ] **Step 3: Build verify**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: compile success.

- [ ] **Step 4: Commit**

```bash
git add host/port/esp-idf/h_port_config.h host/api/src/esp_wifi_weak.c
git commit -m "feat: migrate Wi-Fi feature flags to h_port_config.h"
```

---

### Task 5: BT feature flag header migration (WP 3c)

**Files:**
- Modify: `host/port/esp-idf/h_port_config.h`
- Modify: `host/drivers/bt/hci_drv.h`

- [ ] **Step 1: Add BT feature flags to `h_port_config.h`**

In `host/port/esp-idf/h_port_config.h`, append before the final `#endif`. IMPORTANT: Preserve legacy behavior — `H_BT_BLUEDROID_USE_VHCI` is always `1` regardless of config:

```c
/* ── BT feature flags (from legacy port_esp_hosted_host_bt_config.h) ── */

#if CONFIG_ESP_HOSTED_CP_TARGET_ESP32
  #if CONFIG_BT_BLE_50_FEATURES_SUPPORTED || CONFIG_BT_NIMBLE_50_FEATURE_SUPPORT
    #error "ESP32 co-processor only supports BLE 4.2"
  #endif
#endif

#if CONFIG_ESP_HOSTED_ENABLE_BT_NIMBLE
  #define H_BT_HOST_ESP_NIMBLE 1
#else
  #define H_BT_HOST_ESP_NIMBLE 0
#endif

#if CONFIG_ESP_HOSTED_NIMBLE_HCI_VHCI
  #define H_BT_USE_VHCI 1
#else
  #define H_BT_USE_VHCI 0
#endif

#if CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
  #define H_BT_HOST_ESP_BLUEDROID 1
#else
  #define H_BT_HOST_ESP_BLUEDROID 0
#endif

/* Legacy: unconditionally 1 (BlueDroid VHCI is always assumed when BlueDroid is enabled).
 * This preserves the existing behavior from port_esp_hosted_host_bt_config.h. */
#define H_BT_BLUEDROID_USE_VHCI 1

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
  #define H_BT_ENABLE_LL_INIT 1
#else
  #define H_BT_ENABLE_LL_INIT 0
#endif

#if H_BT_HOST_ESP_NIMBLE && H_BT_HOST_ESP_BLUEDROID
  #error "Enable only NimBLE or BlueDroid, not both"
#endif
```

- [ ] **Step 2: Update `hci_drv.h` include**

In `host/drivers/bt/hci_drv.h`, change:
```c
#include "port_esp_hosted_host_bt_config.h"
```
to:
```c
#include "h_port_config.h"
```

- [ ] **Step 3: Build verify**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: compile success. BT feature flag values unchanged.

- [ ] **Step 4: Commit**

```bash
git add host/port/esp-idf/h_port_config.h host/drivers/bt/hci_drv.h
git commit -m "feat: migrate BT feature flags to h_port_config.h (preserve behavior)"
```

---

### Task 6: SPI bus helper migration with new naming (WP 4 — SPI)

**Files:**
- Modify: `host/port/esp-idf/h_transport_spi_bus.h`
- Modify: `host/port/esp-idf/h_transport_spi_bus.c`
- Modify: `host/port/esp-idf/h_transport_spi.c`

Complete migration table for SPI:

| Legacy function (port_esp_hosted_host_spi.c) | New function (h_transport_spi_bus.c) |
|---|---|
| `hosted_spi_init()` | `h_spi_bus_init()` |
| `hosted_spi_deinit()` | `h_spi_bus_deinit()` |
| `hosted_do_spi_transfer()` | `h_spi_bus_transfer()` |

Additional symbols declared in `transport_drv_api.h`:
- `esp_hosted_tx()` — defined in `spi_drv.c`, now declared in `transport_drv_api.h`
- `ensure_slave_bus_ready()` — defined in `spi_drv.c` (shared), now declared in `transport_drv_api.h`

- [ ] **Step 1: Write `h_transport_spi_bus.h`**

Replace the content of `host/port/esp-idf/h_transport_spi_bus.h`:

```c
/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_SPI_BUS_H
#define H_TRANSPORT_SPI_BUS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SPI bus helpers — ESP-IDF port internal API.
 * Replaces legacy hosted_spi_*() from port_esp_hosted_host_spi.c. */

void *h_spi_bus_init(void);
int   h_spi_bus_deinit(void *handle);
int   h_spi_bus_transfer(void *trans);

#ifdef __cplusplus
}
#endif

#endif /* H_TRANSPORT_SPI_BUS_H */
```

- [ ] **Step 2: Write `h_transport_spi_bus.c`**

Replace the content of `host/port/esp-idf/h_transport_spi_bus.c` with the complete SPI bus implementation. Extract the body of `hosted_spi_init()`, `hosted_spi_deinit()`, and `hosted_do_spi_transfer()` from `host/port/esp/freertos/src/port_esp_hosted_host_spi.c`, rename them, and replace OSAL macros:

```c
/*
 * SPDX-FileCopyrightText: 2015-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "h_transport_spi_bus.h"
#include "h_wrapper.h"
#include "h_port_config.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "transport_drv.h"

#define TAG "h_spi_bus"

#ifdef CONFIG_IDF_TARGET_ESP32
  #define SENDER_HOST  HSPI_HOST
#else
  #define SENDER_HOST  SPI2_HOST
#endif

extern void *spi_handle;

void *h_spi_bus_init(void)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "Transport: SPI, Mode:%u Freq:%uMHz TxQ:%u RxQ:%u\n"
             " GPIOs: CLK:%u MOSI:%u MISO:%u CS:%u HS:%u DR:%u SlaveReset:%u",
            H_SPI_MODE, H_SPI_FD_CLK_MHZ, H_SPI_TX_Q, H_SPI_RX_Q,
            H_GPIO_SCLK_Pin, H_GPIO_MOSI_Pin, H_GPIO_MISO_Pin,
            H_GPIO_CS_Pin, H_GPIO_HANDSHAKE_Pin, H_GPIO_DATA_READY_Pin,
            H_GPIO_PIN_RESET);

    spi_device_handle_t *h = h_calloc(1, sizeof(spi_device_handle_t));
    assert(h);
    spi_handle = h;

    spi_bus_config_t buscfg = {
        .mosi_io_num = H_GPIO_MOSI_Pin,
        .miso_io_num = H_GPIO_MISO_Pin,
        .sclk_io_num = H_GPIO_SCLK_Pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
#ifdef CONFIG_IDF_TARGET_ESP32P4
        .clock_source = SPI_CLK_SRC_SPLL,
#endif
        .clock_speed_hz = MHZ_TO_HZ(H_SPI_FD_CLK_MHZ),
        .duty_cycle_pos = 128,
        .mode = H_SPI_MODE,
        .spics_io_num = H_GPIO_CS_Pin,
        .cs_ena_posttrans = 3,
        .queue_size = 3,
    };

    ret = spi_bus_initialize(SENDER_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
    ret = spi_bus_add_device(SENDER_HOST, &devcfg, h);
    assert(ret == ESP_OK);

    gpio_set_drive_capability(H_GPIO_CS_Pin, GPIO_DRIVE_CAP_3);
    gpio_set_drive_capability(H_GPIO_SCLK_Pin, GPIO_DRIVE_CAP_3);
    return h;
}

int h_spi_bus_deinit(void *handle)
{
    if (!handle) {
        ESP_LOGE(TAG, "Invalid handle for SPI deinit");
        return -1;
    }

    spi_device_handle_t *spi_dev_handle = (spi_device_handle_t *)handle;
    esp_err_t ret = spi_bus_remove_device(*spi_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove SPI device: %d", ret);
        return -1;
    }

    ret = spi_bus_free(SENDER_HOST);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to free SPI bus: %d", ret);
        return -1;
    }

    h_free(handle);
    spi_handle = NULL;
    ESP_LOGI(TAG, "SPI deinitialized");
    return 0;
}

int h_spi_bus_transfer(void *trans)
{
    spi_transaction_t t = {0};
    struct hosted_transport_context_t *spi_trans = trans;

    t.length = spi_trans->tx_buf_size * 8;
    t.tx_buffer = spi_trans->tx_buf;
    t.rx_buffer = spi_trans->rx_buf;
    t.flags |= SPI_TRANS_DMA_BUFFER_ALIGN_MANUAL;

    return spi_device_transmit(*((spi_device_handle_t *)spi_handle), &t);
}
```

- [ ] **Step 3: Update `h_transport_spi.c` — switch to new bus functions**

In `host/port/esp-idf/h_transport_spi.c`:
- Delete the `extern hosted_spi_init()` / `extern hosted_spi_deinit()` / `extern hosted_do_spi_transfer()` declarations
- Delete the `extern ensure_slave_bus_ready()` declaration
- Delete the `extern esp_hosted_tx(...)` declaration
- Add: `#include "h_transport_spi_bus.h"`
- Add: `#include "transport_drv_api.h"`
- Update adapter functions:
  - `hosted_spi_init()` → `h_spi_bus_init()`
  - `hosted_spi_deinit(handle)` → `h_spi_bus_deinit(handle)`
  - `hosted_do_spi_transfer(transfer_ctx)` → `h_spi_bus_transfer(transfer_ctx)`
- `ensure_slave_bus_ready` and `esp_hosted_tx` keep their names (now declared in `transport_drv_api.h` instead of extern)

- [ ] **Step 4: Clean up adapter file top comments**

In `host/port/esp-idf/h_transport_spi.c`, update the top comment block (lines 1-9) to remove references to `host/port/esp/freertos/`. Specifically remove:
```
 * Adapts the existing hosted_spi_* implementations from
 * host/port/esp/freertos/src/port_esp_hosted_host_spi.c
 * (for bus init/deinit/transfer) and
 * host/port/esp/freertos/src/port_esp_hosted_host_os.c
 * (for GPIO) to the h_transport_contract_t vtable.
```
Replace with a brief description that does not reference legacy paths, e.g.:
```
 * ESP-IDF SPI Transport port — bus adapter + contract assembly.
```

This avoids the WP8 `rg -rn 'port/esp/freertos'` audit failing on migrated files. Apply the same cleanup to `h_transport_spi_hd.c`, `h_transport_sdio.c`, and `h_transport_uart.c` in their respective tasks.

- [ ] **Step 5: Build verify SPI**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: compile success.

- [ ] **Step 6: Commit**

```bash
git add host/port/esp-idf/h_transport_spi_bus.h host/port/esp-idf/h_transport_spi_bus.c \
        host/port/esp-idf/h_transport_spi.c
git commit -m "feat: migrate SPI bus helpers to h_transport_spi_bus.c"
```

---

### Task 7: SPI-HD bus helper migration with new naming (WP 4 — SPI-HD)

**Files:**
- Modify: `host/port/esp-idf/h_transport_spi_hd_bus.h`
- Modify: `host/port/esp-idf/h_transport_spi_hd_bus.c`
- Modify: `host/port/esp-idf/h_transport_spi_hd.c`

Complete migration table for SPI-HD:

| Legacy function (port_esp_hosted_host_spi_hd.c) | New function (h_transport_spi_hd_bus.c) |
|---|---|
| `hosted_spi_hd_init()` | `h_spi_hd_bus_init()` |
| `hosted_spi_hd_deinit()` | `h_spi_hd_bus_deinit()` |
| `hosted_spi_hd_read_reg()` | `h_spi_hd_bus_read_reg()` |
| `hosted_spi_hd_write_reg()` | `h_spi_hd_bus_write_reg()` |
| `hosted_spi_hd_read_dma()` | `h_spi_hd_bus_read_dma()` |
| `hosted_spi_hd_write_dma()` | `h_spi_hd_bus_write_dma()` |
| `hosted_spi_hd_send_cmd9()` | `h_spi_hd_bus_send_cmd9()` |
| `hosted_spi_hd_set_data_lines()` | **keeps name** (weak/strong exception — see below) |
| `spi_hd_rx_tx_flags` (static var) | → `h_transport_spi_hd_bus.c` |

Additional: `ensure_slave_bus_ready()` and `esp_hosted_tx()` now via `transport_drv_api.h`.

**Weak/strong exception for `hosted_spi_hd_set_data_lines()`:**

This function is different from the other bus helpers. It's accessed through the OSAL contract chain:
```
h_transport_drv.c → h_spi_hd_set_data_lines() (h_wrapper.h macro)
  → g_h_osal.spi_hd_set_data_lines → h_spi_hd_set_data_lines_adapter (h_osal.c)
    → hosted_spi_hd_set_data_lines()
```

`h_osal.c` defines `hosted_spi_hd_set_data_lines()` as `__attribute__((weak))` (returns `H_ERR_NOT_SUP`). The legacy `port_esp_hosted_host_spi_hd.c` provides the strong definition. When both are compiled (SPI-HD enabled), the linker picks the strong one.

To preserve this pattern without duplicate symbols:
- **Keep the name `hosted_spi_hd_set_data_lines()`** — move the strong definition + `spi_hd_rx_tx_flags` static var to `h_transport_spi_hd_bus.c`
- The weak stub remains in `h_osal.c` for non-SPI-HD transports
- No caller changes needed — `h_osal.c`'s adapter already calls `hosted_spi_hd_set_data_lines()`

This is the **only exception** to the "new naming" rule in WP 4. The weak/strong pattern already prevents duplicate symbol issues.

- [ ] **Step 1: Write `h_transport_spi_hd_bus.h`**

Declare 7 renamed functions + 1 kept-name function with `H_TRANSPORT_SPI_HD_BUS_H` guard:
```c
void *h_spi_hd_bus_init(void);
int   h_spi_hd_bus_deinit(void *ctx);
int   h_spi_hd_bus_read_reg(uint32_t reg, uint32_t *data, int poll, bool lock_required);
int   h_spi_hd_bus_write_reg(uint32_t reg, uint32_t *data, bool lock_required);
int   h_spi_hd_bus_read_dma(uint8_t *data, uint16_t size, bool lock_required);
int   h_spi_hd_bus_write_dma(uint8_t *data, uint16_t size, bool lock_required);
int   h_spi_hd_bus_send_cmd9(void);

/* Weak/strong exception — keeps legacy name (see task notes) */
int   hosted_spi_hd_set_data_lines(uint32_t data_lines);
```

- [ ] **Step 2: Write `h_transport_spi_hd_bus.c`**

Extract all function bodies + `spi_hd_rx_tx_flags` static var from `host/port/esp/freertos/src/port_esp_hosted_host_spi_hd.c`. Rename 7 functions. For `hosted_spi_hd_set_data_lines()`, keep the name and move the implementation and `spi_hd_rx_tx_flags` variable as-is. Replace `HOSTED_CREATE_HANDLE` → `h_calloc`, `g_h.funcs->_h_*` → `h_*` wrappers, `HOSTED_FREE` → `h_free`.

- [ ] **Step 3: Update `h_transport_spi_hd.c`**

Delete all `extern hosted_spi_hd_*()` / `extern ensure_slave_bus_ready()` / `extern esp_hosted_tx()` declarations. Add `#include "h_transport_spi_hd_bus.h"` and `#include "transport_drv_api.h"`. Update all adapter calls to new function names.

- [ ] **Step 4: Build verify**

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" -B build-spi_hd build
```

Expected: compile success.

- [ ] **Step 5: Commit**

```bash
git add host/port/esp-idf/h_transport_spi_hd_bus.h \
        host/port/esp-idf/h_transport_spi_hd_bus.c \
        host/port/esp-idf/h_transport_spi_hd.c
git commit -m "feat: migrate SPI-HD bus helpers to h_transport_spi_hd_bus.c"
```

---

### Task 8: SDIO bus helper migration with new naming (WP 4 — SDIO)

**Files:**
- Modify: `host/port/esp-idf/h_transport_sdio_bus.h`
- Modify: `host/port/esp-idf/h_transport_sdio_bus.c`
- Modify: `host/port/esp-idf/h_transport_sdio.c`

Complete migration table for SDIO:

| Legacy function (port_esp_hosted_host_sdio.c) | New function (h_transport_sdio_bus.c) |
|---|---|
| `hosted_sdio_init()` | `h_sdio_bus_init()` |
| `hosted_sdio_deinit()` | `h_sdio_bus_deinit()` |
| `hosted_sdio_card_init()` | `h_sdio_bus_card_init()` |
| `hosted_sdio_card_deinit()` | `h_sdio_bus_card_deinit()` |
| `hosted_sdio_read_reg()` | `h_sdio_bus_read_reg()` |
| `hosted_sdio_write_reg()` | `h_sdio_bus_write_reg()` |
| `hosted_sdio_read_block()` | `h_sdio_bus_read_block()` |
| `hosted_sdio_write_block()` | `h_sdio_bus_write_block()` |
| `hosted_sdio_wait_slave_intr()` | `h_sdio_bus_wait_intr()` |

Additional: `ensure_slave_bus_ready()` and `esp_hosted_tx()` now via `transport_drv_api.h`.

- [ ] **Step 1: Write `h_transport_sdio_bus.h`**

Declare all 9 functions with `H_TRANSPORT_SDIO_BUS_H` guard.

- [ ] **Step 2: Write `h_transport_sdio_bus.c`**

Extract all 9 function bodies from `host/port/esp/freertos/src/port_esp_hosted_host_sdio.c`. Rename them. Replace `g_h.funcs->_h_*` → `h_*` wrappers, `HOSTED_FREE` → `h_free`, `HOSTED_CALLOC` → `h_calloc`.

- [ ] **Step 3: Update `h_transport_sdio.c`**

Delete all `extern hosted_sdio_*()` / `extern ensure_slave_bus_ready()` / `extern esp_hosted_tx()` declarations. Add `#include "h_transport_sdio_bus.h"` and `#include "transport_drv_api.h"`. Update all adapter calls to new function names.

- [ ] **Step 4: Build verify**

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio" -B build-sdio build
```

Expected: compile success.

- [ ] **Step 5: Commit**

```bash
git add host/port/esp-idf/h_transport_sdio_bus.h \
        host/port/esp-idf/h_transport_sdio_bus.c \
        host/port/esp-idf/h_transport_sdio.c
git commit -m "feat: migrate SDIO bus helpers to h_transport_sdio_bus.c"
```

---

### Task 9: UART bus helper migration with new naming (WP 4 — UART)

**Files:**
- Modify: `host/port/esp-idf/h_transport_uart_bus.h`
- Modify: `host/port/esp-idf/h_transport_uart_bus.c`
- Modify: `host/port/esp-idf/h_transport_uart.c`

Complete migration table for UART:

| Legacy function (port_esp_hosted_host_uart.c) | New function (h_transport_uart_bus.c) |
|---|---|
| `hosted_uart_init()` | `h_uart_bus_init()` |
| `hosted_uart_deinit()` | `h_uart_bus_deinit()` |
| `hosted_uart_read()` | `h_uart_bus_read()` |
| `hosted_uart_write()` | `h_uart_bus_write()` |
| `hosted_uart_flush_input()` | `h_uart_bus_flush_input()` |

Additional: `ensure_slave_bus_ready()` and `esp_hosted_tx()` now via `transport_drv_api.h`.

- [ ] **Step 1: Write `h_transport_uart_bus.h`**

Declare all 5 functions with `H_TRANSPORT_UART_BUS_H` guard.

- [ ] **Step 2: Write `h_transport_uart_bus.c`**

Extract all 5 function bodies from `host/port/esp/freertos/src/port_esp_hosted_host_uart.c`. Rename them. Replace `g_h.funcs->_h_*` → `h_*` wrappers.

- [ ] **Step 3: Update `h_transport_uart.c`**

Delete all `extern hosted_uart_*()` / `extern ensure_slave_bus_ready()` / `extern esp_hosted_tx()` declarations. Add `#include "h_transport_uart_bus.h"` and `#include "transport_drv_api.h"`. Update all adapter calls to new function names.

- [ ] **Step 4: Build verify**

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart" -B build-uart build
```

Expected: compile success.

- [ ] **Step 5: Commit**

```bash
git add host/port/esp-idf/h_transport_uart_bus.h \
        host/port/esp-idf/h_transport_uart_bus.c \
        host/port/esp-idf/h_transport_uart.c
git commit -m "feat: migrate UART bus helpers to h_transport_uart_bus.c"
```

---

### Task 10: Clean up `h_transport_drv.h` — move decls to `transport_drv_api.h` (WP 4 tail)

**Files:**
- Modify: `host/core/include/h_internal/h_transport_drv.h`

- [ ] **Step 1: Remove duplicate declarations, add include**

In `host/core/include/h_internal/h_transport_drv.h`:
- Delete the `esp_hosted_tx()` declaration — now in `transport_drv_api.h`
- Delete the `check_if_max_freq_used()` declaration — now in `transport_drv_api.h`
- Delete the `ensure_slave_bus_ready()` declaration — now in `transport_drv_api.h`
- Add `#include "transport_drv_api.h"` after the existing includes

- [ ] **Step 2: Build verify**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: compile success.

- [ ] **Step 3: Commit**

```bash
git add host/core/include/h_internal/h_transport_drv.h
git commit -m "refactor: move driver API decls from h_transport_drv.h to transport_drv_api.h"
```

---

### Task 11: OpenThread util migration (WP 4 — additional)

**Files:**
- Modify: `host/port/esp-idf/h_port_config.h`
- Modify: `host/port/esp-idf/h_transport_common.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add `H_OT_*` macros to `h_port_config.h`**

In `host/port/esp-idf/h_port_config.h`, append before the final `#endif`. Copy from `port_esp_hosted_host_openthread.h`:

```c
/* ── OpenThread feature flags (from legacy port_esp_hosted_host_openthread.h) ── */

#if CONFIG_ESP_HOSTED_OT_HOST_ENABLE
  #define H_OT_HOST_ENABLE 1
#else
  #define H_OT_HOST_ENABLE 0
#endif

#if CONFIG_ESP_HOSTED_OT_TRANSPORT_UART
  #define H_OT_TRANSPORT_UART_DEDICATED 1
#else
  #define H_OT_TRANSPORT_UART_DEDICATED 0
#endif

#if CONFIG_ESP_HOSTED_OT_TRANSPORT_HOSTED
  #define H_OT_TRANSPORT_HOSTED 1
#else
  #define H_OT_TRANSPORT_HOSTED 0
#endif

#if H_OT_TRANSPORT_UART_DEDICATED
  #define H_OT_UART_PORT          CONFIG_ESP_HOSTED_OT_UART_PORT
  #define H_OT_PIN_TO_RCP_TX      CONFIG_ESP_HOSTED_OT_PIN_TO_RCP_TX
  #define H_OT_PIN_TO_RCP_RX      CONFIG_ESP_HOSTED_OT_PIN_TO_RCP_RX
  #define H_OT_UART_BAUDRATE      CONFIG_ESP_HOSTED_OT_UART_BAUDRATE
  #define H_OT_UART_NUM_DATA_BITS CONFIG_ESP_HOSTED_OT_UART_NUM_DATA_BITS
  #define H_OT_UART_PARITY        CONFIG_ESP_HOSTED_OT_UART_PARITY
  #define H_OT_UART_STOP_BITS     CONFIG_ESP_HOSTED_OT_UART_STOP_BITS
#endif
```

- [ ] **Step 2: Migrate `esp_hosted_openthread_get_radio_config()` to `h_transport_common.c`**

Copy the function body from `host/port/esp/freertos/src/port_esp_hosted_host_ot_util.c`. Changes:
- `#include "port_esp_hosted_host_config.h"` → `#include "h_port_config.h"`
- `#include "port_esp_hosted_host_openthread.h"` → (delete — macros now in `h_port_config.h`)
- Wrap with `#if H_OT_HOST_ENABLE` … `#endif` compile guard so the file compiles when OT is disabled
- Keep function name `esp_hosted_openthread_get_radio_config()` unchanged (public API)

```c
/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "h_port_config.h"

#if H_OT_HOST_ENABLE

#include "driver/uart.h"
#include "esp_hosted_openthread.h"
#include <esp_log.h>

static const char TAG[] = "h_ot_util";

esp_err_t esp_hosted_openthread_get_radio_config(esp_hosted_openthread_radio_config_t *config)
{
    if (!config)
        return ESP_FAIL;

#if H_OT_TRANSPORT_UART_DEDICATED
    ESP_LOGD(TAG, "returning dedicated OpenThread UART config");
    config->type = HOSTED_OPENTHREAD_TRANSPORT_UART;
    esp_hosted_openthread_uart_config_t *uart_config = &config->radio_uart_config;

    uart_config->port       = H_OT_UART_PORT;
    uart_config->baud_rate  = H_OT_UART_BAUDRATE;
    uart_config->data_bits  = H_OT_UART_NUM_DATA_BITS;
    uart_config->parity     = H_OT_UART_PARITY;
    uart_config->stop_bits  = H_OT_UART_STOP_BITS;
    uart_config->flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    uart_config->rx_flow_ctrl_thresh = 0;
    uart_config->source_clk = UART_SCLK_DEFAULT;
    uart_config->rx_pin     = H_OT_PIN_TO_RCP_TX;
    uart_config->tx_pin     = H_OT_PIN_TO_RCP_RX;

    return ESP_OK;
#endif
#if H_OT_TRANSPORT_HOSTED
    #error OpenThread over ESP-Hosted transport not yet supported
    return ESP_FAIL;
#endif
}

#endif /* H_OT_HOST_ENABLE */
```

- [ ] **Step 3: Atomic CMake swap**

In `CMakeLists.txt`, in the same commit:
- Remove `"${host_dir}/port/esp/freertos/src/port_esp_hosted_host_ot_util.c"` from legacy port srcs
- Remove the `if(CONFIG_ESP_HOSTED_OT_HOST_ENABLE)` block that adds it
- Confirm `h_transport_common.c` is in the main srcs list (added in Task 1)

- [ ] **Step 4: Build verify**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" -B build-spi build
```

Expected: compile success with and without `CONFIG_ESP_HOSTED_OT_HOST_ENABLE`.

- [ ] **Step 5: Commit**

```bash
git add host/port/esp-idf/h_port_config.h \
        host/port/esp-idf/h_transport_common.c \
        CMakeLists.txt
git commit -m "feat: migrate OpenThread util and H_OT_* config to h_port_config.h"
```

---

### Task 12: Active file old include replacement (WP 5)

**Files:**
- Modify: `host/api/src/esp_hosted_transport_config.c`
- Modify: `host/api/src/esp_hosted_api.c`
- Modify: `host/drivers/transport/spi/spi_drv.c`
- Modify: `host/drivers/transport/spi_hd/spi_hd_drv.c`
- Modify: `host/drivers/transport/sdio/sdio_drv.c`
- Modify: `host/drivers/transport/uart/uart_drv.c`
- Modify: `host/drivers/transport/sdio/sdio_reg.h`
- Modify: `host/drivers/rpc/slaveif/rpc_slave_if.h`
- Modify: `host/drivers/rpc/wrap/rpc_wrap.h`
- Modify: `host/drivers/bt/hci_stub_drv.c`

- [ ] **Step 1: Replace old includes in each file**

| File:line | Old include | Replace with |
|---|---|---|
| `host/api/src/esp_hosted_transport_config.c:10` | `#include "port_esp_hosted_host_config.h"` | `#include "h_port_config.h"` |
| `host/api/src/esp_hosted_api.c:15` | `#include "port_esp_hosted_host_wifi_config.h"` | (delete — `h_port_config.h` already included at line 17) |
| `host/api/src/esp_hosted_api.c:16` | `#include "port_esp_hosted_host_openthread.h"` | (delete — macros now in `h_port_config.h`) |
| `host/drivers/transport/spi/spi_drv.c:20` | `#include "port_esp_hosted_host_config.h"` | `#include "h_port_config.h"` |
| `host/drivers/transport/spi/spi_drv.c:22` | `#include "port_esp_hosted_host_os.h"` | (delete — no longer needed) |
| `host/drivers/transport/spi_hd/spi_hd_drv.c:24` | `#include "port_esp_hosted_host_config.h"` | `#include "h_port_config.h"` |
| `host/drivers/transport/spi_hd/spi_hd_drv.c:31` | `#include "port_esp_hosted_host_os.h"` | (delete) |
| `host/drivers/transport/sdio/sdio_drv.c:88` | `#include "port_esp_hosted_host_config.h"` | `#include "h_port_config.h"` |
| `host/drivers/transport/uart/uart_drv.c:21` | `#include "port_esp_hosted_host_os.h"` | `#include "h_port_config.h"` |
| `host/drivers/transport/sdio/sdio_reg.h:11` | `#include "port_esp_hosted_host_config.h"` | `#include "h_port_config.h"` |
| `host/drivers/rpc/slaveif/rpc_slave_if.h:18` | `#include "port_esp_hosted_host_config.h"` | `#include "h_port_config.h"` |
| `host/drivers/rpc/wrap/rpc_wrap.h:19` | `#include "port_esp_hosted_host_wifi_config.h"` | `#include "h_port_config.h"` |
| `host/drivers/rpc/wrap/rpc_wrap.h:20` | `#include "port_esp_hosted_host_config.h"` | (delete — duplicates h_port_config.h) |
| `host/drivers/rpc/wrap/rpc_wrap.h:21` | `#include "port_esp_hosted_host_openthread.h"` | (delete — macros now in h_port_config.h) |
| `host/drivers/rpc/wrap/rpc_wrap.h:24` | `#include "port_esp_hosted_host_wifi_config.h"` (else branch) | `#include "h_port_config.h"` |
| `host/drivers/bt/hci_stub_drv.c:18` | `#include "port_esp_hosted_host_os.h"` | `#include "h_port_config.h"` (add `#include "h_types.h"` if not present) |

Note: `rpc_wrap.h` is an active header (in `priv_include`, line 58 of CMakeLists.txt, and included by `esp_hosted_api.c`). Its `rpc_wrap.c` is NOT compiled, but removing the legacy include path will break compilation of files that include `rpc_wrap.h`.

- [ ] **Step 2: Build verify all four transports**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi"    -B build-spi    build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" -B build-spi_hd build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio"   -B build-sdio   build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart"   -B build-uart   build
```

Expected: all four compile successfully.

- [ ] **Step 3: Commit**

```bash
git add host/api/src/esp_hosted_transport_config.c \
        host/api/src/esp_hosted_api.c \
        host/drivers/transport/spi/spi_drv.c \
        host/drivers/transport/spi_hd/spi_hd_drv.c \
        host/drivers/transport/sdio/sdio_drv.c \
        host/drivers/transport/sdio/sdio_reg.h \
        host/drivers/transport/uart/uart_drv.c \
        host/drivers/rpc/slaveif/rpc_slave_if.h \
        host/drivers/rpc/wrap/rpc_wrap.h \
        host/drivers/bt/hci_stub_drv.c
git commit -m "refactor: replace legacy port includes with h_port_config.h"
```

---

### Task 13: Remove legacy port from CMakeLists (WP 6)

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Remove legacy port srcs and priv_include from CMakeLists**

In `CMakeLists.txt`:
- Delete lines 120-121 (the `priv_include` and comment for legacy port):
  ```cmake
  list(APPEND priv_include "${host_dir}/port/esp/freertos/include")
  ```
- Delete lines 122-126 (the legacy port srcs — note `port_esp_hosted_host_transport_defaults.c` was already removed in Task 3):
  ```cmake
  list(APPEND srcs
      "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_os.c"
      "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_transport_defaults.c"
  )
  ```
  If `port_esp_hosted_host_transport_defaults.c` was already removed, only the comment and `port_esp_hosted_host_os.c` remain.

- Delete lines 128-136 (per-transport legacy port conditionals):
  ```cmake
  if(CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE)
      list(APPEND srcs "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_sdio.c")
  elseif(CONFIG_ESP_HOSTED_SPI_HD_HOST_INTERFACE)
      list(APPEND srcs "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_spi_hd.c")
  elseif(CONFIG_ESP_HOSTED_SPI_HOST_INTERFACE)
      list(APPEND srcs "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_spi.c")
  elseif(CONFIG_ESP_HOSTED_UART_HOST_INTERFACE)
      list(APPEND srcs "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_uart.c")
  endif()
  ```

- Delete lines 137-140 (OpenThread utility — already migrated in Task 11):
  ```cmake
  if(CONFIG_ESP_HOSTED_OT_HOST_ENABLE)
      list(APPEND srcs "${host_dir}/port/esp/freertos/src/port_esp_hosted_host_ot_util.c")
  endif()
  ```

- [ ] **Step 2: Build verify all four transports**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi"    -B build-spi    build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" -B build-spi_hd build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio"   -B build-sdio   build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart"   -B build-uart   build
```

Expected: all four compile. `host/port/esp/freertos/` no longer in build.

- [ ] **Step 3: Commit**

```bash
git add CMakeLists.txt
git commit -m "feat: remove legacy port from ESP-IDF host active source set"
```

---

### Task 14: Transport compile check full matrix (WP 7)

- [ ] **Step 1: Run four-transport build matrix**

```bash
cd examples/host_compile_check
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi"    -B build-spi    build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" -B build-spi_hd build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio"   -B build-sdio   build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart"   -B build-uart   build
```

Expected: All four PASS.

- [ ] **Step 2: Run optional feature builds (non-blocking)**

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.power_save" -B build-ps build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.bt_vhci"    -B build-bt build
```

---

### Task 15: Symbol audit (WP 8)

- [ ] **Step 1: Run legacy symbol audit on active source set**

```bash
ACTIVE_DIRS="host/api/src host/core/src host/drivers/serial host/drivers/bt \
  host/drivers/transport/spi host/drivers/transport/sdio \
  host/drivers/transport/spi_hd host/drivers/transport/uart \
  host/drivers/power_save host/drivers/virtual_serial_if \
  host/drivers/rpc/slaveif host/port/esp-idf \
  common/utils common/proto common/mempool common/protobuf-c"

# Also audit the RPC wrap header (active via priv_include) but NOT rpc_wrap.c (not compiled)
ACTIVE_HEADERS="host/drivers/rpc/wrap/rpc_wrap.h host/drivers/rpc/slaveif/rpc_slave_if.h"

rg -n 'g_h\.funcs|->_h_|HOSTED_CONFIG_INIT_DEFAULT' $ACTIVE_DIRS $ACTIVE_HEADERS
rg -n 'HOSTED_FREE|HOSTED_CALLOC|HOSTED_MALLOC|HOSTED_FREE_HANDLE|H_DEFLT_FREE_FUNC' $ACTIVE_DIRS $ACTIVE_HEADERS
rg -n '#include "port_esp_hosted_host_' $ACTIVE_DIRS $ACTIVE_HEADERS
rg -n 'extern.*esp_hosted_tx' $ACTIVE_DIRS $ACTIVE_HEADERS
rg -n '\bg_h\b' $ACTIVE_DIRS $ACTIVE_HEADERS
rg -n 'struct hosted_config_t g_h|\bg_h\s*\.\s*funcs\b|\bHOSTED_CONFIG_INIT_DEFAULT\b' $ACTIVE_DIRS $ACTIVE_HEADERS
rg -rn 'port/esp/freertos' $ACTIVE_DIRS $ACTIVE_HEADERS host/core/include
```

Expected: All zero matches.

**Allowlist item — `hosted_spi_hd_set_data_lines`:**

This is the only legacy-named function that remains in the active source set. It uses a weak/strong pattern: `h_osal.c` provides the weak stub, `h_transport_spi_hd_bus.c` provides the strong definition. When SPI-HD is not the active transport, the weak stub correctly returns `H_ERR_NOT_SUP`. The name is preserved because the function is accessed through the OSAL contract chain (`h_spi_hd_set_data_lines()` wrapper macro → `g_h_osal.spi_hd_set_data_lines` → adapter → `hosted_spi_hd_set_data_lines()`). Changing the name would require changing the OSAL contract, which is out of scope for this migration. This exception is documented in the 24 号 spec and this plan.

- [ ] **Step 2: Verify core isolation**

```bash
bash scripts/check_core_isolation.sh
```

Expected: `12/12 = 100% PASS`

- [ ] **Step 3: Verify pre-commit**

```bash
pre-commit run --all-files
```

Expected: all hooks pass, or document pre-existing failures.

---

### Task 16: Hardware smoke test (WP 9 — enhanced, non-blocking)

- [ ] **Step 1: Flash and test SPI transport**

If hardware available: build/flash SPI slave + host, verify init → scan → connect → data plane.

If NOT available: document `compile + static only`.

---

### Task 17: Documentation update (WP 10)

**Files:**
- Modify: `docs/felix/current_platform_active_path_matrix.md`
- Modify: `docs/felix/24.Legacy Port收口退场-执行方案.md`

- [ ] **Step 1: Update active path matrix**

In `docs/felix/current_platform_active_path_matrix.md`:
- Remove the legacy-helper classification
- Remove all `host/port/esp/freertos/` entries
- Add new bus files under required-active
- Update completion status

- [ ] **Step 2: Add execution results to 24号文档**

Add an "执行结果" section summarizing each WP status, build results, and audit results.

- [ ] **Step 3: Commit**

```bash
git add docs/felix/current_platform_active_path_matrix.md \
        docs/felix/24.Legacy\ Port收口退场-执行方案.md
git commit -m "docs: finalize legacy port retirement documentation"
```
