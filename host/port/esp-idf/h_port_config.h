/* host/port/esp-idf/h_port_config.h
 * Platform configuration for ESP-IDF — maps Kconfig options to host port defines. */

#ifndef H_PORT_CONFIG_ESPIDF_H
#define H_PORT_CONFIG_ESPIDF_H

#include <stddef.h>    /* NULL */

#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif

#include "esp_idf_version.h"
#include "esp_hosted_transport.h"  /* ESP_TRANSPORT_*_MAX_BUF_SIZE */

/* Keep this header self-contained for ESP-IDF port files that include it
 * directly before h_config.h has defined the transport enum constants. */
#ifndef H_TRANSPORT_NONE
#define H_TRANSPORT_NONE    0
#endif
#ifndef H_TRANSPORT_SDIO
#define H_TRANSPORT_SDIO    1
#endif
#ifndef H_TRANSPORT_SPI_HD
#define H_TRANSPORT_SPI_HD  2
#endif
#ifndef H_TRANSPORT_SPI
#define H_TRANSPORT_SPI     3
#endif
#ifndef H_TRANSPORT_UART
#define H_TRANSPORT_UART    4
#endif

/* ── Transport — selected by Kconfig at build time ── */
#if defined(CONFIG_ESP_HOSTED_SPI_HOST_INTERFACE)
  #define H_TRANSPORT_IN_USE  H_TRANSPORT_SPI
#elif defined(CONFIG_ESP_HOSTED_SPI_HD_HOST_INTERFACE)
  #define H_TRANSPORT_IN_USE  H_TRANSPORT_SPI_HD
#elif defined(CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE)
  #define H_TRANSPORT_IN_USE  H_TRANSPORT_SDIO
#elif defined(CONFIG_ESP_HOSTED_UART_HOST_INTERFACE)
  #define H_TRANSPORT_IN_USE  H_TRANSPORT_UART
#else
  #error "No ESP-Hosted transport selected in Kconfig"
#endif

/* ── Platform Identity ── */
#define H_PORT_NAME         "esp-idf"
#define H_PORT_VERSION      "5.3.2"
#define H_PORT_RTOS         "freertos"
#define H_PORT_RTOS_VER     "10.5.1"
#define H_PORT_CHIP         CONFIG_IDF_TARGET
#define H_PORT_BUILD_DATE   __DATE__

/* ── IDF version ── */
#define H_IDF_VERSION_MAJOR 5
#define H_IDF_VERSION_MINOR 3

/* ESP-IDF uses static netif creation (esp_netif_create_default_wifi_*) */
#undef H_HOST_USES_STATIC_NETIF
#define H_HOST_USES_STATIC_NETIF 1

/* ── Thread defaults (Kconfig with fallback to legacy values) ── */
#ifndef H_DEFAULT_TASK_STACK
  #define H_DEFAULT_TASK_STACK   (5*1024)
#endif
#ifndef H_DEFAULT_TASK_PRIO
  #define H_DEFAULT_TASK_PRIO    23
#endif
#define H_DEFAULT_RPC_TASK_STACK  (5*1024)

/* ── Common GPIO / misc (was in port_esp_hosted_host_os.h / host_config.h) ── */
#ifdef H_ENABLE
#undef H_ENABLE
#endif
#ifdef H_DISABLE
#undef H_DISABLE
#endif
#ifdef H_GPIO_MODE_INPUT
#undef H_GPIO_MODE_INPUT
#endif
#ifdef H_GPIO_MODE_OUTPUT
#undef H_GPIO_MODE_OUTPUT
#endif
#ifdef H_GPIO_PULL_UP
#undef H_GPIO_PULL_UP
#endif
#ifdef H_GPIO_PULL_DOWN
#undef H_GPIO_PULL_DOWN
#endif
#ifdef ESP_HOSTED_SDIO_UNRESPONSIVE_CODE
#undef ESP_HOSTED_SDIO_UNRESPONSIVE_CODE
#endif
#define H_ENABLE   1
#define H_DISABLE  0
#define H_GPIO_MODE_INPUT    1
#define H_GPIO_MODE_OUTPUT   2
#define H_GPIO_PULL_UP       1
#define H_GPIO_PULL_DOWN     0
#define ESP_HOSTED_SDIO_UNRESPONSIVE_CODE 0x107
#ifdef H_GPIO_PIN_RESET
#undef H_GPIO_PIN_RESET
#endif
#ifdef CONFIG_ESP_HOSTED_GPIO_SLAVE_RESET_SLAVE
  #define H_GPIO_PIN_RESET   CONFIG_ESP_HOSTED_GPIO_SLAVE_RESET_SLAVE
#else
  #define H_GPIO_PIN_RESET   (-1)
#endif
#ifdef H_GPIO_PORT_RESET
#undef H_GPIO_PORT_RESET
#endif
#define H_GPIO_PORT_RESET  NULL

/* ── Transport buffer size (per-transport, from Kconfig) ── */
#if   H_TRANSPORT_IN_USE == H_TRANSPORT_SPI
  #define H_MAX_TRANSPORT_BUFFER_SIZE  ESP_TRANSPORT_SPI_MAX_BUF_SIZE
#elif H_TRANSPORT_IN_USE == H_TRANSPORT_SDIO
  #define H_MAX_TRANSPORT_BUFFER_SIZE  ESP_TRANSPORT_SDIO_MAX_BUF_SIZE
#elif H_TRANSPORT_IN_USE == H_TRANSPORT_SPI_HD
  #define H_MAX_TRANSPORT_BUFFER_SIZE  ESP_TRANSPORT_SPI_HD_MAX_BUF_SIZE
#elif H_TRANSPORT_IN_USE == H_TRANSPORT_UART
  #define H_MAX_TRANSPORT_BUFFER_SIZE  ESP_TRANSPORT_UART_MAX_BUF_SIZE
#endif

/* ── Host Power-Save Kconfig mappings (was in port_esp_hosted_host_config.h) ── */
#ifdef H_HOST_PS_ALLOWED
#undef H_HOST_PS_ALLOWED
#endif
#ifdef H_HOST_WAKEUP_GPIO
#undef H_HOST_WAKEUP_GPIO
#endif
#ifdef H_HOST_WAKEUP_GPIO_PORT
#undef H_HOST_WAKEUP_GPIO_PORT
#endif
#ifdef H_HOST_WAKEUP_GPIO_LEVEL
#undef H_HOST_WAKEUP_GPIO_LEVEL
#endif
#ifdef CONFIG_ESP_HOSTED_HOST_POWER_SAVE_ENABLED
  #define H_HOST_PS_ALLOWED  1
  #define H_HOST_WAKEUP_GPIO  CONFIG_ESP_HOSTED_HOST_WAKEUP_GPIO
  #define H_HOST_WAKEUP_GPIO_PORT  NULL
  #ifdef CONFIG_ESP_HOSTED_HOST_WAKEUP_GPIO_LEVEL
    #define H_HOST_WAKEUP_GPIO_LEVEL  CONFIG_ESP_HOSTED_HOST_WAKEUP_GPIO_LEVEL
  #else
    #define H_HOST_WAKEUP_GPIO_LEVEL  1
  #endif
#else
  #define H_HOST_PS_ALLOWED  0
  #define H_HOST_WAKEUP_GPIO  (-1)
  #define H_HOST_WAKEUP_GPIO_PORT  NULL
  #define H_HOST_WAKEUP_GPIO_LEVEL  1
#endif

/* ── Phase 2 feature flags — explicitly 0 for Phase 1 ── */
#define H_FEATURE_BLUETOOTH  0
#define H_FEATURE_OTA        0
#define H_FEATURE_NETSPLIT   0

/* ── ESP-IDF version-gated Wi-Fi features (replacing
 *   port_esp_hosted_host_wifi_config.h in core layer code) ── */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
  #define H_PRESENT_IN_ESP_IDF_5_4_0      1
#else
  #define H_PRESENT_IN_ESP_IDF_5_4_0      0
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
  #define H_WIFI_NEW_RESERVED_FIELD_NAMES 1
  #define H_PRESENT_IN_ESP_IDF_5_5_0      1
#else
  #define H_WIFI_NEW_RESERVED_FIELD_NAMES 0
  #define H_PRESENT_IN_ESP_IDF_5_5_0      0
#endif

#ifdef CONFIG_ESP_HOSTED_DECODE_WIFI_RESERVED_FIELD
  #define H_DECODE_WIFI_RESERVED_FIELD 1
#else
  #define H_DECODE_WIFI_RESERVED_FIELD 0
#endif

/* ── Transport config ── */
#ifdef H_WIFI_TX_DATA_THROTTLE_LOW_THRESHOLD
#undef H_WIFI_TX_DATA_THROTTLE_LOW_THRESHOLD
#endif
#ifdef H_WIFI_TX_DATA_THROTTLE_HIGH_THRESHOLD
#undef H_WIFI_TX_DATA_THROTTLE_HIGH_THRESHOLD
#endif
#ifdef CONFIG_HOST_TO_ESP_WIFI_DATA_THROTTLE
  #define H_WIFI_TX_DATA_THROTTLE_LOW_THRESHOLD        CONFIG_ESP_HOSTED_TO_WIFI_DATA_THROTTLE_LOW_THRESHOLD
  #define H_WIFI_TX_DATA_THROTTLE_HIGH_THRESHOLD       CONFIG_ESP_HOSTED_TO_WIFI_DATA_THROTTLE_HIGH_THRESHOLD
#else
  #define H_WIFI_TX_DATA_THROTTLE_LOW_THRESHOLD        0
  #define H_WIFI_TX_DATA_THROTTLE_HIGH_THRESHOLD       0
#endif

#ifdef H_TEST_RAW_TP
#undef H_TEST_RAW_TP
#endif
#ifdef H_RAW_TP_REPORT_INTERVAL
#undef H_RAW_TP_REPORT_INTERVAL
#endif
#ifdef H_RAW_TP_PKT_LEN
#undef H_RAW_TP_PKT_LEN
#endif
#ifdef H_TEST_RAW_TP_DIR
#undef H_TEST_RAW_TP_DIR
#endif
#if CONFIG_ESP_HOSTED_RAW_THROUGHPUT_TRANSPORT
  #define H_TEST_RAW_TP  1
  #define H_RAW_TP_REPORT_INTERVAL  CONFIG_ESP_HOSTED_RAW_TP_REPORT_INTERVAL
  #define H_RAW_TP_PKT_LEN          CONFIG_ESP_HOSTED_RAW_TP_HOST_TO_ESP_PKT_LEN
  #if CONFIG_ESP_HOSTED_RAW_THROUGHPUT_TX_TO_SLAVE
    #define H_TEST_RAW_TP_DIR 0x04  /* ESP_TEST_RAW_TP__HOST_TO_ESP */
  #elif CONFIG_ESP_HOSTED_RAW_THROUGHPUT_RX_FROM_SLAVE
    #define H_TEST_RAW_TP_DIR 0x02  /* ESP_TEST_RAW_TP__ESP_TO_HOST */
  #elif CONFIG_ESP_HOSTED_RAW_THROUGHPUT_BIDIRECTIONAL
    #define H_TEST_RAW_TP_DIR 0x08  /* ESP_TEST_RAW_TP__BIDIRECTIONAL */
  #else
    #define H_TEST_RAW_TP_DIR 0
  #endif
#else
  #define H_TEST_RAW_TP  0
  #define H_RAW_TP_REPORT_INTERVAL  0
  #define H_RAW_TP_PKT_LEN          0
  #define H_TEST_RAW_TP_DIR 0
#endif

#ifdef ESP_PKT_STATS
#undef ESP_PKT_STATS
#endif
#ifdef ESP_PKT_STATS_REPORT_INTERVAL
#undef ESP_PKT_STATS_REPORT_INTERVAL
#endif
#ifdef CONFIG_ESP_HOSTED_PKT_STATS
  #define ESP_PKT_STATS  1
  #define ESP_PKT_STATS_REPORT_INTERVAL  CONFIG_ESP_HOSTED_PKT_STATS_INTERVAL_SEC
#else
  #define ESP_PKT_STATS  0
  #define ESP_PKT_STATS_REPORT_INTERVAL  0
#endif

#ifdef H_MEM_MONITOR
#undef H_MEM_MONITOR
#endif
#ifdef CONFIG_ESP_HOSTED_MEM_MONITOR
  #define H_MEM_MONITOR  1
#else
  #define H_MEM_MONITOR  0
#endif

#ifndef H_MEM_STATS
  #define H_MEM_STATS  0
#endif

#ifndef CONFIG_H_LOWER_MEMCOPY
  #define CONFIG_H_LOWER_MEMCOPY  0
#endif

#ifndef H_SEC_TO_MILLISEC
  #define H_SEC_TO_MILLISEC(x)  (1000U * (x))
#endif

#endif /* H_PORT_CONFIG_ESPIDF_H */
