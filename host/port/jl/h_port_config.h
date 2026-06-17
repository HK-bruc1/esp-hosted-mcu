/* host/port/jl/h_port_config.h
 * JL (AC701N) port configuration for ESP-Hosted-MCU Host framework.
 */

#ifndef H_PORT_CONFIG_JL_H
#define H_PORT_CONFIG_JL_H

#define H_PORT_NAME         "jl-ac701n"
#define H_PORT_VERSION      "0.1.0"
#define H_PORT_RTOS         "JL-RTOS"
#define H_PORT_RTOS_VER     "1.0"
#define H_PORT_CHIP         "AC701N"
#define H_PORT_BUILD_DATE   __DATE__

/* Phase 1: UART only */
#define H_TRANSPORT_IN_USE  H_TRANSPORT_UART

/* Thread defaults — tune against JL RTOS stack unit */
#define H_DEFAULT_TASK_STACK      4096
#define H_DEFAULT_TASK_PRIO       5
#define H_DEFAULT_RPC_TASK_STACK  H_DEFAULT_TASK_STACK

/* Transport buffer */
#define H_MAX_TRANSPORT_BUFFER_SIZE  1600

/* Phase 1 features disabled */
#define H_FEATURE_BLUETOOTH 0
#define H_FEATURE_OTA       0
#define H_FEATURE_NETSPLIT  0
#define H_FEATURE_DPP       0
#define H_FEATURE_ENTERPRISE 0

/* Non-ESP-IDF: version-gated features absent */
#define H_PRESENT_IN_ESP_IDF_5_4_0  0
#define H_PRESENT_IN_ESP_IDF_5_5_0  0
#define H_DECODE_WIFI_RESERVED_FIELD 0
#define H_WIFI_NEW_RESERVED_FIELD_NAMES 0

/* netif: JL has no esp_netif in Phase 1 */
#define H_HOST_USES_STATIC_NETIF 0

/* Wi-Fi/NVS portable mappings */
#define H_WIFI_AUTO_CONNECT_ON_STA_START  0
#define H_WIFI_NVS_ENABLED                0
#define H_FW_VERSION_MISMATCH_WARNING_SUPPRESS  0
#define H_PEER_DATA_TRANSFER              0
#define H_MAX_CUSTOM_MSG_HANDLERS         0

/* GPIO values */
#define H_GPIO_LOW   0
#define H_GPIO_HIGH  1
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

/* Reset pin (adjust for actual hardware) */
#define H_GPIO_PORT_RESET  NULL
#define H_GPIO_PIN_RESET   25
#define H_RESET_ACTIVE_HIGH  1
#if H_RESET_ACTIVE_HIGH
  #define H_RESET_VAL_ACTIVE   H_GPIO_HIGH
  #define H_RESET_VAL_INACTIVE H_GPIO_LOW
#else
  #define H_RESET_VAL_ACTIVE   H_GPIO_LOW
  #define H_RESET_VAL_INACTIVE H_GPIO_HIGH
#endif

/* Transport switch flags */
#define H_SPI_HD_HOST_INTERFACE    0
#define H_UART_HOST_TRANSPORT      1

/* UART configuration */
#define H_UART_PORT                   0
#define H_UART_BAUD_RATE              115200
#define H_UART_NUM_DATA_BITS          8
#define H_UART_PARITY                 0
#define H_UART_START_BITS             1
#define H_UART_STOP_BITS              1
#define H_UART_FLOWCTRL               0   /* disable in Phase 1 */
#define H_UART_CLK_SRC                0
#define H_UART_CHECKSUM               1
#define H_UART_PIN_TX                 0   /* adjust for hardware */
#define H_UART_PORT_TX                NULL
#define H_UART_PIN_RX                 1   /* adjust for hardware */
#define H_UART_PORT_RX                NULL
#define H_UART_TX_QUEUE_SIZE          10
#define H_UART_RX_QUEUE_SIZE          10
#define H_TRANSPORT_QUEUE_SIZE        H_UART_TX_QUEUE_SIZE

/* Buffer macros used by transport task layer */
#define MAX_TRANSPORT_BUFFER_SIZE  H_MAX_TRANSPORT_BUFFER_SIZE
#define MAX_UART_BUFFER_SIZE       H_MAX_TRANSPORT_BUFFER_SIZE
#define H_ESP_PAYLOAD_HEADER_OFFSET  12
#define MAX_PAYLOAD_SIZE           (MAX_TRANSPORT_BUFFER_SIZE - H_ESP_PAYLOAD_HEADER_OFFSET)

/* mempool disabled in Phase 1 */
#define H_USE_MEMPOOL  0
#define CONFIG_ESP_HOSTED_USE_MEMPOOL  0

/* zerocopy flags */
#define H_BUFF_NO_ZEROCOPY  0
#define H_BUFF_ZEROCOPY     1

/* Phase 2 feature flags explicitly disabled */
#define H_TEST_RAW_TP              0
#define H_RAW_TP_REPORT_INTERVAL   0
#define H_RAW_TP_PKT_LEN           0
#define H_TEST_RAW_TP_DIR          0
#define ESP_PKT_STATS              0

/* Wi-Fi TX throttle thresholds (disabled in Phase 1) */
#ifndef H_WIFI_TX_DATA_THROTTLE_LOW_THRESHOLD
#define H_WIFI_TX_DATA_THROTTLE_LOW_THRESHOLD   0
#endif
#ifndef H_WIFI_TX_DATA_THROTTLE_HIGH_THRESHOLD
#define H_WIFI_TX_DATA_THROTTLE_HIGH_THRESHOLD  0
#endif
#define ESP_PKT_STATS_REPORT_INTERVAL 0
#define H_MEM_MONITOR              0
#define H_MEM_STATS                0
#define CONFIG_H_LOWER_MEMCOPY     0

/* Power-save disabled */
#define H_HOST_PS_ALLOWED                 0
#define H_HOST_WAKEUP_GPIO                (-1)
#define H_HOST_WAKEUP_GPIO_PORT           NULL
#define H_HOST_WAKEUP_GPIO_LEVEL          1
#define H_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE 0
#define H_HOST_RESTART_NO_COMMUNICATION_WITH_SLAVE_TIMEOUT_MS  -1
#define H_TRANSPORT_RESTART_ON_FAILURE    0
#define H_SLAVE_RESET_ON_EVERY_HOST_BOOTUP 1
#define H_SLAVE_RESET_ONLY_IF_NECESSARY   0
#define H_HOST_SDIO_RESET_DELAY_MS        1500

/* OpenThread disabled */
#define H_OT_HOST_ENABLE          0
#define H_OT_TRANSPORT_UART_DEDICATED 0
#define H_OT_TRANSPORT_HOSTED     0

/* BT disabled */
#define H_BT_HOST_ESP_NIMBLE      0
#define H_BT_HOST_ESP_BLUEDROID   0
#define H_BT_USE_VHCI             0
#define H_BT_BLUEDROID_USE_VHCI   0
#define H_BT_ENABLE_LL_INIT       0

/* Wi-Fi feature flags */
#define H_WIFI_HE_SUPPORT         0
#define H_WIFI_HE_GREATER_THAN_ESP_IDF_5_3 0
#define H_WIFI_DUALBAND_SUPPORT   0
#define H_WIFI_ENTERPRISE_SUPPORT 0
#define H_GOT_TWT_ENABLE_KEEP_ALIVE 0
#define H_GOT_AP_CONFIG_PARAM_TRANSITION_DISABLE 0
#define H_PRESENT_IN_ESP_IDF_6_0_0 0
#define H_GOT_SET_EAP_METHODS_API 0
#define H_GOT_EAP_SET_DOMAIN_NAME 0
#define H_GOT_EAP_OKC_SUPPORT     0
#define H_DPP_SUPPORT             0

/* Logging tag helper */
#ifndef DEFINE_LOG_TAG
#define DEFINE_LOG_TAG(sTr) static const char TAG[] = #sTr
#endif

/* Weak reference attribute */
#ifndef H_WEAK_REF
#define H_WEAK_REF __attribute__((weak))
#endif

#endif /* H_PORT_CONFIG_JL_H */
