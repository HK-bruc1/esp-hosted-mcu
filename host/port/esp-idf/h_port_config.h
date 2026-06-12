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
#include "esp_heap_caps.h"         /* heap_caps_get_largest_free_block (for MEM_DUMP) */
#include "esp_system.h"            /* esp_get_*_heap_size (for MEM_DUMP) */

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

/* Co-processor target selection (from legacy port_esp_hosted_host_config.h) */
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32
  #define H_SLAVE_TARGET_ESP32 1
#else
  #define H_SLAVE_TARGET_ESP32 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32S2) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32S2
  #define H_SLAVE_TARGET_ESP32S2 1
#else
  #define H_SLAVE_TARGET_ESP32S2 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32C3) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32C3
  #define H_SLAVE_TARGET_ESP32C3 1
#else
  #define H_SLAVE_TARGET_ESP32C3 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32S3) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32S3
  #define H_SLAVE_TARGET_ESP32S3 1
#else
  #define H_SLAVE_TARGET_ESP32S3 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32C2) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32C2
  #define H_SLAVE_TARGET_ESP32C2 1
#else
  #define H_SLAVE_TARGET_ESP32C2 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32C6) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32C6
  #define H_SLAVE_TARGET_ESP32C6 1
#else
  #define H_SLAVE_TARGET_ESP32C6 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32C5) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32C5
  #define H_SLAVE_TARGET_ESP32C5 1
#else
  #define H_SLAVE_TARGET_ESP32C5 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32C61) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32C61
  #define H_SLAVE_TARGET_ESP32C61 1
#else
  #define H_SLAVE_TARGET_ESP32C61 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32H2) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32H2
  #define H_SLAVE_TARGET_ESP32H2 1
#else
  #define H_SLAVE_TARGET_ESP32H2 0
#endif
#if defined(CONFIG_ESP_HOSTED_CP_TARGET_ESP32H4) && CONFIG_ESP_HOSTED_CP_TARGET_ESP32H4
  #define H_SLAVE_TARGET_ESP32H4 1
#else
  #define H_SLAVE_TARGET_ESP32H4 0
#endif

#if H_SLAVE_TARGET_ESP32 + H_SLAVE_TARGET_ESP32S2 + H_SLAVE_TARGET_ESP32C3 + \
    H_SLAVE_TARGET_ESP32S3 + H_SLAVE_TARGET_ESP32C2 + H_SLAVE_TARGET_ESP32C6 + \
    H_SLAVE_TARGET_ESP32C5 + H_SLAVE_TARGET_ESP32C61 + H_SLAVE_TARGET_ESP32H2 + \
    H_SLAVE_TARGET_ESP32H4 != 1
  #error "No Slave Target or more than one Slave Target was defined."
#endif

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
#define H_GPIO_INTR_DISABLE  0
#define H_GPIO_INTR_POSEDGE  1
#define H_GPIO_INTR_NEGEDGE  2
#define H_GPIO_INTR_ANYEDGE  3
#define H_GPIO_INTR_LOW_LEVEL  4
#define H_GPIO_INTR_HIGH_LEVEL 5
#define ESP_HOSTED_SDIO_UNRESPONSIVE_CODE 0x107

/* Log tag helper (replaces legacy port_esp_hosted_host_log.h DEFINE_LOG_TAG) */
#ifndef DEFINE_LOG_TAG
#define DEFINE_LOG_TAG(sTr) static const char TAG[] = #sTr
#endif

/* Weak reference attribute (replaces legacy port_esp_hosted_host_config.h H_WEAK_REF) */
#ifndef H_WEAK_REF
#define H_WEAK_REF __attribute__((weak))
#endif

/* ── Legacy OS shim macros (from port_esp_hosted_host_os.h) ── */
#define FAST_RAM_ATTR       IRAM_ATTR
typedef int                gpio_pin_state_t;

/* MEM_DUMP — memory diagnostics, uses TAG defined by DEFINE_LOG_TAG */
#ifndef MEM_DUMP
#define MEM_DUMP(s) \
    ESP_LOGD(TAG, "%s free:%lu min-free:%lu lfb-def:%u lfb-8bit:%u", s, \
             (unsigned long int)esp_get_free_heap_size(), \
             (unsigned long int)esp_get_minimum_free_heap_size(), \
             heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT), \
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))
#endif

#ifndef MAX_TRANSPORT_BUFFER_SIZE
#define MAX_TRANSPORT_BUFFER_SIZE  H_MAX_TRANSPORT_BUFFER_SIZE
#endif

#ifndef H_ESP_PAYLOAD_HEADER_OFFSET
#ifdef ESP_PKT_NUM_DEBUG
#define H_ESP_PAYLOAD_HEADER_OFFSET 14
#else
#define H_ESP_PAYLOAD_HEADER_OFFSET 12
#endif
#endif

#ifndef MAX_PAYLOAD_SIZE
#define MAX_PAYLOAD_SIZE \
    (MAX_TRANSPORT_BUFFER_SIZE - H_ESP_PAYLOAD_HEADER_OFFSET)
#endif
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-transport Kconfig → macro mappings
 * (was in legacy port_esp_hosted_host_config.h)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── Common GPIO / utility values (used by per-transport logic) ── */
#ifndef H_GPIO_LOW
#define H_GPIO_LOW   0
#endif
#ifndef H_GPIO_HIGH
#define H_GPIO_HIGH  1
#endif

/* ── Common GPIO port defaults (overridable per-transport) ── */
#ifndef H_GPIO_SCLK_Port
#define H_GPIO_SCLK_Port       NULL
#endif
#ifndef H_GPIO_MOSI_Port
#define H_GPIO_MOSI_Port       NULL
#endif
#ifndef H_GPIO_MISO_Port
#define H_GPIO_MISO_Port       NULL
#endif
#ifndef H_GPIO_CS_Port
#define H_GPIO_CS_Port         NULL
#endif
#ifndef H_GPIO_HANDSHAKE_Port
#define H_GPIO_HANDSHAKE_Port  NULL
#endif
#ifndef H_GPIO_DATA_READY_Port
#define H_GPIO_DATA_READY_Port NULL
#endif

/*
 * One and only one transport is selected at build time via Kconfig.
 * Use a single if/elif chain so H_SPI_HD_HOST_INTERFACE and
 * H_UART_HOST_TRANSPORT are always defined (0 or 1).
 */
#if   H_TRANSPORT_IN_USE == H_TRANSPORT_SPI
/* ══════════════════════════ SPI transport ══════════════════════════ */

  #ifdef CONFIG_ESP_HOSTED_HS_ACTIVE_LOW
    #define H_HANDSHAKE_ACTIVE_HIGH  0
  #else
    /* Default HS: Active High */
    #define H_HANDSHAKE_ACTIVE_HIGH  1
  #endif

  #ifdef CONFIG_ESP_HOSTED_DR_ACTIVE_LOW
    #define H_DATAREADY_ACTIVE_HIGH  0
  #else
    /* Default DR: Active High */
    #define H_DATAREADY_ACTIVE_HIGH  1
  #endif

  #if H_HANDSHAKE_ACTIVE_HIGH
    #define H_HS_VAL_ACTIVE          H_GPIO_HIGH
    #define H_HS_VAL_INACTIVE        H_GPIO_LOW
    #define H_HS_INTR_EDGE           H_GPIO_INTR_POSEDGE
  #else
    #define H_HS_VAL_ACTIVE          H_GPIO_LOW
    #define H_HS_VAL_INACTIVE        H_GPIO_HIGH
    #define H_HS_INTR_EDGE           H_GPIO_INTR_NEGEDGE
  #endif

  #if H_DATAREADY_ACTIVE_HIGH
    #define H_DR_VAL_ACTIVE          H_GPIO_HIGH
    #define H_DR_VAL_INACTIVE        H_GPIO_LOW
    #define H_DR_INTR_EDGE           H_GPIO_INTR_POSEDGE
  #else
    #define H_DR_VAL_ACTIVE          H_GPIO_LOW
    #define H_DR_VAL_INACTIVE        H_GPIO_HIGH
    #define H_DR_INTR_EDGE           H_GPIO_INTR_NEGEDGE
  #endif

  #define H_GPIO_HANDSHAKE_Pin       CONFIG_ESP_HOSTED_SPI_GPIO_HANDSHAKE
  #define H_GPIO_DATA_READY_Pin      CONFIG_ESP_HOSTED_SPI_GPIO_DATA_READY
  #define H_GPIO_MOSI_Pin            CONFIG_ESP_HOSTED_SPI_GPIO_MOSI
  #define H_GPIO_MISO_Pin            CONFIG_ESP_HOSTED_SPI_GPIO_MISO
  #define H_GPIO_SCLK_Pin            CONFIG_ESP_HOSTED_SPI_GPIO_CLK
  #define H_GPIO_CS_Pin              CONFIG_ESP_HOSTED_SPI_GPIO_CS

  #define H_SPI_TX_Q                 CONFIG_ESP_HOSTED_SPI_TX_Q_SIZE
  #define H_SPI_RX_Q                 CONFIG_ESP_HOSTED_SPI_RX_Q_SIZE
  #define H_SPI_MODE                 CONFIG_ESP_HOSTED_SPI_MODE
  #define H_SPI_FD_CLK_MHZ           CONFIG_ESP_HOSTED_SPI_CLK_FREQ
  #define H_TRANSPORT_QUEUE_SIZE     CONFIG_ESP_HOSTED_SPI_TX_Q_SIZE

  #define H_SPI_HD_HOST_INTERFACE    0
  #define H_UART_HOST_TRANSPORT      0

#elif H_TRANSPORT_IN_USE == H_TRANSPORT_SDIO
/* ══════════════════════════ SDIO transport ══════════════════════════ */

  #ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
    #define H_SDIO_SOC_USE_GPIO_MATRIX
  #endif

  #define H_SDIO_CLOCK_FREQ_KHZ      CONFIG_ESP_HOSTED_SDIO_CLOCK_FREQ_KHZ
  #define H_SDIO_BUS_WIDTH           CONFIG_ESP_HOSTED_SDIO_BUS_WIDTH
  #define H_SDMMC_HOST_SLOT          CONFIG_ESP_HOSTED_SDIO_SLOT

  #define H_SDIO_PORT_CLK            NULL
  #define H_SDIO_PORT_CMD            NULL
  #define H_SDIO_PORT_D0             NULL
  #define H_SDIO_PORT_D1             NULL
  #define H_SDIO_PORT_D2             NULL
  #define H_SDIO_PORT_D3             NULL

  #ifdef H_SDIO_SOC_USE_GPIO_MATRIX
    #define H_SDIO_PIN_CLK           CONFIG_ESP_HOSTED_SDIO_PIN_CLK
    #define H_SDIO_PIN_CMD           CONFIG_ESP_HOSTED_SDIO_PIN_CMD
    #define H_SDIO_PIN_D0            CONFIG_ESP_HOSTED_SDIO_PIN_D0
    #define H_SDIO_PIN_D1            CONFIG_ESP_HOSTED_SDIO_PIN_D1
    #if (H_SDIO_BUS_WIDTH == 4)
      #define H_SDIO_PIN_D2          CONFIG_ESP_HOSTED_SDIO_PIN_D2
      #define H_SDIO_PIN_D3          CONFIG_ESP_HOSTED_SDIO_PIN_D3
    #else
      #define H_SDIO_PIN_D2          -1
      #define H_SDIO_PIN_D3          -1
    #endif
  #else
    #define H_SDIO_PIN_CLK           -1
    #define H_SDIO_PIN_CMD           -1
    #define H_SDIO_PIN_D0            -1
    #define H_SDIO_PIN_D1            -1
    #if (H_SDIO_BUS_WIDTH == 4)
      #define H_SDIO_PIN_D2          -1
      #define H_SDIO_PIN_D3          -1
    #else
      #define H_SDIO_PIN_D2          -1
      #define H_SDIO_PIN_D3          -1
    #endif
  #endif

  #define H_SDIO_TX_Q                CONFIG_ESP_HOSTED_SDIO_TX_Q_SIZE
  #define H_SDIO_RX_Q                CONFIG_ESP_HOSTED_SDIO_RX_Q_SIZE
  #define H_SDIO_CHECKSUM            CONFIG_ESP_HOSTED_SDIO_CHECKSUM

  #define H_SDIO_HOST_STREAMING_MODE                1
  #define H_SDIO_ALWAYS_HOST_RX_MAX_TRANSPORT_SIZE  2
  #define H_SDIO_OPTIMIZATION_RX_NONE               3

  #ifdef CONFIG_ESP_HOSTED_SDIO_OPTIMIZATION_RX_STREAMING_MODE
    #define H_SDIO_HOST_RX_MODE      H_SDIO_HOST_STREAMING_MODE
  #elif defined(CONFIG_ESP_HOSTED_SDIO_OPTIMIZATION_RX_MAX_SIZE)
    #define H_SDIO_HOST_RX_MODE      H_SDIO_ALWAYS_HOST_RX_MAX_TRANSPORT_SIZE
  #else
    /* Use this if unsure */
    #define H_SDIO_HOST_RX_MODE      H_SDIO_OPTIMIZATION_RX_NONE
  #endif

  /* Pad transfer len for host operation */
  #define H_SDIO_TX_LEN_TO_TRANSFER(x)   (((x) + 3) & (~3))
  #define H_SDIO_RX_LEN_TO_TRANSFER(x)   (((x) + 3) & (~3))

  /* Do Block Mode only transfers
   *
   * When enabled, SDIO only uses block mode transfers for higher
   * throughput. Data lengths are padded to multiples of ESP_BLOCK_SIZE.
   *
   * This is safe for the SDIO slave:
   * - for Host Tx: slave will ignore extra data sent by Host
   * - for Host Rx: slave will send extra 0 data, ignored by Host
   */
  #define H_SDIO_TX_BLOCK_ONLY_XFER  1
  #define H_SDIO_RX_BLOCK_ONLY_XFER  1

  /* workarounds for some SDIO transfer errors that may occur.
   * Below workarounds could be enabled for non-ESP MCUs to test first.
   * Once everything is stable, can disable workarounds and test again.
   */
  #if 0
    #define H_SDIO_TX_LIMIT_XFER_SIZE_WORKAROUND
    #define H_SDIO_RX_LIMIT_XFER_SIZE_WORKDAROUND
  #endif

  #if defined(H_SDIO_TX_LIMIT_XFER_SIZE_WORKAROUND)
    #define H_SDIO_TX_BLOCKS_TO_TRANSFER(x)   (1)
  #else
    #define H_SDIO_TX_BLOCKS_TO_TRANSFER(x)   ((x) / ESP_BLOCK_SIZE)
  #endif

  #if defined(H_SDIO_RX_LIMIT_XFER_SIZE_WORKDAROUND)
    #define H_SDIO_RX_BLOCKS_TO_TRANSFER(x)   (1)
  #else
    #define H_SDIO_RX_BLOCKS_TO_TRANSFER(x)   ((x) / ESP_BLOCK_SIZE)
  #endif

  #define H_TRANSPORT_QUEUE_SIZE      CONFIG_ESP_HOSTED_SDIO_TX_Q_SIZE

  #define H_SPI_HD_HOST_INTERFACE     0
  #define H_UART_HOST_TRANSPORT       0

#elif H_TRANSPORT_IN_USE == H_TRANSPORT_SPI_HD
/* ═════════════════════════ SPI-HD transport ═════════════════════════ */

  #define H_SPI_HD_HOST_INTERFACE     1

  #define H_SPI_HD_CONFIG_2_DATA_LINES  0
  #define H_SPI_HD_CONFIG_4_DATA_LINES  1

  #if CONFIG_ESP_HOSTED_SPI_HD_DR_ACTIVE_HIGH
    #define H_SPI_HD_DATAREADY_ACTIVE_HIGH  1
  #else
    #define H_SPI_HD_DATAREADY_ACTIVE_HIGH  0
  #endif

  #if H_SPI_HD_DATAREADY_ACTIVE_HIGH
    #define H_SPI_HD_DR_VAL_ACTIVE        H_GPIO_HIGH
    #define H_SPI_HD_DR_VAL_INACTIVE      H_GPIO_LOW
    #define H_SPI_HD_DR_INTR_EDGE         H_GPIO_INTR_POSEDGE
  #else
    #define H_SPI_HD_DR_VAL_ACTIVE        H_GPIO_LOW
    #define H_SPI_HD_DR_VAL_INACTIVE      H_GPIO_HIGH
    #define H_SPI_HD_DR_INTR_EDGE         H_GPIO_INTR_NEGEDGE
  #endif

  #define H_SPI_HD_HOST_NUM_DATA_LINES  CONFIG_ESP_HOSTED_SPI_HD_INTERFACE_NUM_DATA_LINES

  #define H_SPI_HD_PORT_D0              NULL
  #define H_SPI_HD_PORT_D1              NULL
  #define H_SPI_HD_PORT_D2              NULL
  #define H_SPI_HD_PORT_D3              NULL
  #define H_SPI_HD_PORT_CS              NULL
  #define H_SPI_HD_PORT_CLK             NULL

  #define H_SPI_HD_PIN_D0               CONFIG_ESP_HOSTED_SPI_HD_GPIO_D0
  #define H_SPI_HD_PIN_D1               CONFIG_ESP_HOSTED_SPI_HD_GPIO_D1
  #if (CONFIG_ESP_HOSTED_SPI_HD_INTERFACE_NUM_DATA_LINES == 4)
    #define H_SPI_HD_PIN_D2             CONFIG_ESP_HOSTED_SPI_HD_GPIO_D2
    #define H_SPI_HD_PIN_D3             CONFIG_ESP_HOSTED_SPI_HD_GPIO_D3
  #else
    #define H_SPI_HD_PIN_D2             -1
    #define H_SPI_HD_PIN_D3             -1
  #endif

  #define H_SPI_HD_PIN_CS               CONFIG_ESP_HOSTED_SPI_HD_GPIO_CS
  #define H_SPI_HD_PIN_CLK              CONFIG_ESP_HOSTED_SPI_HD_GPIO_CLK
  #define H_SPI_HD_PORT_DATA_READY      NULL
  #define H_SPI_HD_PIN_DATA_READY       CONFIG_ESP_HOSTED_SPI_HD_GPIO_DATA_READY

  #define H_SPI_HD_CLK_MHZ              CONFIG_ESP_HOSTED_SPI_HD_CLK_FREQ
  #define H_SPI_HD_MODE                 CONFIG_ESP_HOSTED_SPI_HD_MODE
  #define H_SPI_HD_TX_QUEUE_SIZE        CONFIG_ESP_HOSTED_SPI_HD_TX_Q_SIZE
  #define H_SPI_HD_RX_QUEUE_SIZE        CONFIG_ESP_HOSTED_SPI_HD_RX_Q_SIZE

  #define H_SPI_HD_CHECKSUM             CONFIG_ESP_HOSTED_SPI_HD_CHECKSUM

  #define H_SPI_HD_NUM_COMMAND_BITS     8
  #define H_SPI_HD_NUM_ADDRESS_BITS     8
  #define H_SPI_HD_NUM_DUMMY_BITS       8

  #define H_TRANSPORT_QUEUE_SIZE        CONFIG_ESP_HOSTED_SPI_HD_TX_Q_SIZE

  #define H_UART_HOST_TRANSPORT         0

#elif H_TRANSPORT_IN_USE == H_TRANSPORT_UART
/* ══════════════════════════ UART transport ══════════════════════════ */

  #include "hal/uart_types.h"

  #define H_UART_HOST_TRANSPORT         1

  #define H_UART_PORT                   CONFIG_ESP_HOSTED_UART_PORT
  #define H_UART_NUM_DATA_BITS          CONFIG_ESP_HOSTED_UART_NUM_DATA_BITS
  #define H_UART_PARITY                 CONFIG_ESP_HOSTED_UART_PARITY
  #define H_UART_START_BITS             1
  #define H_UART_STOP_BITS              CONFIG_ESP_HOSTED_UART_STOP_BITS
  #define H_UART_FLOWCTRL               UART_HW_FLOWCTRL_DISABLE
  #define H_UART_CLK_SRC                UART_SCLK_DEFAULT

  #define H_UART_CHECKSUM               CONFIG_ESP_HOSTED_UART_CHECKSUM
  #define H_UART_BAUD_RATE              CONFIG_ESP_HOSTED_UART_BAUDRATE
  #define H_UART_PIN_TX                 CONFIG_ESP_HOSTED_UART_PIN_TX
  #define H_UART_PORT_TX                NULL
  #define H_UART_PIN_RX                 CONFIG_ESP_HOSTED_UART_PIN_RX
  #define H_UART_PORT_RX                NULL
  #define H_UART_TX_QUEUE_SIZE          CONFIG_ESP_HOSTED_UART_TX_Q_SIZE
  #define H_UART_RX_QUEUE_SIZE          CONFIG_ESP_HOSTED_UART_RX_Q_SIZE

  #define H_TRANSPORT_QUEUE_SIZE        CONFIG_ESP_HOSTED_UART_TX_Q_SIZE

  #define H_SPI_HD_HOST_INTERFACE        0

#endif /* per-transport selection */

/* ── Reset pin logic (used by all transports) ── */
#ifdef CONFIG_ESP_HOSTED_RESET_GPIO_ACTIVE_LOW
  #define H_RESET_ACTIVE_HIGH          0
#else
  #define H_RESET_ACTIVE_HIGH          1
#endif

#if H_RESET_ACTIVE_HIGH
  #define H_RESET_VAL_ACTIVE           H_GPIO_HIGH
  #define H_RESET_VAL_INACTIVE         H_GPIO_LOW
#else
  #define H_RESET_VAL_ACTIVE           H_GPIO_LOW
  #define H_RESET_VAL_INACTIVE         H_GPIO_HIGH
#endif

/* ── Slave reset strategy ── */
#if defined(CONFIG_ESP_HOSTED_TRANSPORT_RESTART_ON_FAILURE)
  #define H_TRANSPORT_RESTART_ON_FAILURE    1
#else
  #define H_TRANSPORT_RESTART_ON_FAILURE    0
#endif

#if defined(CONFIG_ESP_HOSTED_SLAVE_RESET_ON_EVERY_HOST_BOOTUP)
  /* Always reset the slave when host boots up.
   * This ensures a clean transport state and prevents any inconsistent states,
   * but causes the slave to reboot every time the host boots up.
   */
  #define H_SLAVE_RESET_ON_EVERY_HOST_BOOTUP  1
  #define H_SLAVE_RESET_ONLY_IF_NECESSARY     0
#elif defined(CONFIG_ESP_HOSTED_SLAVE_RESET_ONLY_IF_NECESSARY)
  /* Only reset the slave if initialization fails.
   * This reduces slave reboots but assumes the slave interface is in a
   * consistent state. If initialization fails, the host will assume the
   * slave is in an inconsistent or deinitialized state and will reset it.
   */
  #define H_SLAVE_RESET_ON_EVERY_HOST_BOOTUP  0
  #define H_SLAVE_RESET_ONLY_IF_NECESSARY     1
#else
  /* Default to always reset for backward compatibility */
  #define H_SLAVE_RESET_ON_EVERY_HOST_BOOTUP  1
  #define H_SLAVE_RESET_ONLY_IF_NECESSARY     0
#endif

#ifdef CONFIG_ESP_HOSTED_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE
  /* Enable host auto-restart if communication with slave is lost.
   * When enabled, the host will reset itself to recover the connection
   * if the slave becomes non-responsive for the configured timeout period.
   * This acts as a safeguard in case the slave does not issue the first event.
   */
  #define H_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE  1
#else
  /* Disable host auto-restart on communication failure */
  #define H_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE  0
#endif

#if defined(CONFIG_ESP_HOSTED_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE_TIMEOUT)
  /* Timeout in milliseconds before host restarts due to no communication.
   * Maximum time that the host will wait for a response from the slave
   * before triggering an automatic restart.
   */
  #define H_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE_TIMEOUT_MS \
      (CONFIG_ESP_HOSTED_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE_TIMEOUT * 1000)
#else
  /* Default timeout value (-1 means disabled) */
  #define H_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE_TIMEOUT_MS  -1
#endif

/* ── SDIO reset delay (adjust if co-processor needs more time after reset) ── */
#if CONFIG_ESP_HOSTED_SDIO_RESET_DELAY_MS
  #define H_HOST_SDIO_RESET_DELAY_MS   CONFIG_ESP_HOSTED_SDIO_RESET_DELAY_MS
#else
  #define H_HOST_SDIO_RESET_DELAY_MS   1500
#endif

/* ── Wi-Fi / NVS / firmware config portability mappings (WP4) ──
 * CONFIG_* → H_* bridges so core layer code never references CONFIG_*
 * directly.  The generic h_port_config.h provides fallback defaults
 * (all 0) for ports that don't override these. */
#ifdef CONFIG_ESP_HOSTED_WIFI_AUTO_CONNECT_ON_STA_START
  #define H_WIFI_AUTO_CONNECT_ON_STA_START  CONFIG_ESP_HOSTED_WIFI_AUTO_CONNECT_ON_STA_START
#else
  #undef H_WIFI_AUTO_CONNECT_ON_STA_START
  #define H_WIFI_AUTO_CONNECT_ON_STA_START  0
#endif

#ifdef CONFIG_ESP_HOSTED_FW_VERSION_MISMATCH_WARNING_SUPPRESS
  #define H_FW_VERSION_MISMATCH_WARNING_SUPPRESS  1
#else
  #undef H_FW_VERSION_MISMATCH_WARNING_SUPPRESS
  #define H_FW_VERSION_MISMATCH_WARNING_SUPPRESS  0
#endif

#ifdef CONFIG_ESP_WIFI_NVS_ENABLED
  #define H_WIFI_NVS_ENABLED  1
#else
  #undef H_WIFI_NVS_ENABLED
  #define H_WIFI_NVS_ENABLED  0
#endif

/* Custom RPC portability mappings.
 * Keep these always defined so core can use value-based #if checks. */
#ifdef CONFIG_ESP_HOSTED_ENABLE_PEER_DATA_TRANSFER
  #define H_PEER_DATA_TRANSFER  1
#else
  #undef H_PEER_DATA_TRANSFER
  #define H_PEER_DATA_TRANSFER  0
#endif

#ifdef CONFIG_ESP_HOSTED_MAX_CUSTOM_MSG_HANDLERS
  #define H_MAX_CUSTOM_MSG_HANDLERS  CONFIG_ESP_HOSTED_MAX_CUSTOM_MSG_HANDLERS
#else
  #undef H_MAX_CUSTOM_MSG_HANDLERS
  #define H_MAX_CUSTOM_MSG_HANDLERS  0
#endif

#endif /* H_PORT_CONFIG_ESPIDF_H */
