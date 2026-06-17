# host/port/jl/jl_port.mk
# Example JL (AC701N) Makefile integration for ESP-Hosted-MCU Host framework.
# Include this file from the top-level SDK Makefile after defining
# ESP_HOSTED_DIR and JL system include paths.

# Use relative paths from JL SDK root so that JL's c_OBJS suffix replacement
# and BUILD_DIR prefix produce valid target names like objs/esp-hosted-mcu/... .c.o
ESP_HOSTED_REL_DIR ?= esp-hosted-mcu

# -----------------------------------------------------------------------------
# Include paths
#   host/port/jl/ MUST be first so shadow headers override ESP-IDF originals.
# -----------------------------------------------------------------------------
ESP_HOSTED_INCLUDES := \
    -I$(ESP_HOSTED_REL_DIR)/host/port/jl \
    -I$(ESP_HOSTED_REL_DIR)/host/port/include \
    -I$(ESP_HOSTED_REL_DIR)/host/api/priv \
    -I$(ESP_HOSTED_REL_DIR)/host/core/include/h_public \
    -I$(ESP_HOSTED_REL_DIR)/host/core/include/h_internal \
    -I$(ESP_HOSTED_REL_DIR)/host/api/include \
    -I$(ESP_HOSTED_REL_DIR)/host \
    -I$(ESP_HOSTED_REL_DIR)/host/drivers/serial \
    -I$(ESP_HOSTED_REL_DIR)/host/drivers/transport \
    -I$(ESP_HOSTED_REL_DIR)/host/drivers/virtual_serial_if \
    -I$(ESP_HOSTED_REL_DIR)/host/drivers/rpc/core \
    -I$(ESP_HOSTED_REL_DIR)/host/drivers/rpc/slaveif \
    -I$(ESP_HOSTED_REL_DIR)/host/drivers/rpc/wrap \
    -I$(ESP_HOSTED_REL_DIR)/common \
    -I$(ESP_HOSTED_REL_DIR)/common/transport \
    -I$(ESP_HOSTED_REL_DIR)/common/proto \
    -I$(ESP_HOSTED_REL_DIR)/common/protobuf-c \
    -I$(ESP_HOSTED_REL_DIR)/common/rpc

# -----------------------------------------------------------------------------
# Core / API / common sources compiled for JL Phase 1 (UART only).
# Excluded: legacy ESP-IDF drivers, BT, OTA, net-split, power-save, mempool.
# -----------------------------------------------------------------------------
ESP_HOSTED_SOURCES := \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/port_init.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_osal.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_event.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_wifi.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_transport_uart.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_transport_uart_bus.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_transport_task.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_transport_defaults.c \
    $(ESP_HOSTED_REL_DIR)/host/port/jl/h_control_serial_adapter.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_init.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_rpc_core.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_rpc_utils.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_rpc_req.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_rpc_rsp.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_rpc_evt.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_rpc_wrap.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_api.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_serial_if.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_transport_drv.c \
    $(ESP_HOSTED_REL_DIR)/host/core/src/h_transport_util.c \
    $(ESP_HOSTED_REL_DIR)/host/drivers/serial/serial_ll_if.c \
    $(ESP_HOSTED_REL_DIR)/host/drivers/virtual_serial_if/serial_if.c \
    $(ESP_HOSTED_REL_DIR)/host/api/src/esp_hosted_api.c \
    $(ESP_HOSTED_REL_DIR)/host/api/src/esp_wifi_weak.c \
    $(ESP_HOSTED_REL_DIR)/common/proto/esp_hosted_rpc.pb-c.c \
    $(ESP_HOSTED_REL_DIR)/common/protobuf-c/protobuf-c/protobuf-c.c

# -----------------------------------------------------------------------------
# Extend JL Makefile variables
# -----------------------------------------------------------------------------
INCLUDES += $(ESP_HOSTED_INCLUDES)
c_SRC_FILES += $(ESP_HOSTED_SOURCES)

# Required JL SDK defines
DEFINES += -DCONFIG_ESP_HOSTED_UART_HOST_INTERFACE=1
DEFINES += -DCONFIG_ESP_HOSTED_ENABLED=1
