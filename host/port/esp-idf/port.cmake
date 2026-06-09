# ESP-IDF port sources
set(ESP_HOSTED_PORT_SRCS
    "${host_dir}/port/esp-idf/port_init.c"
    "${host_dir}/port/esp-idf/h_osal.c"
    "${host_dir}/port/esp-idf/h_event.c"
    "${host_dir}/port/esp-idf/h_wifi_type_adapt.c"
    "${host_dir}/port/esp-idf/h_control_serial_adapter.c"
    "${host_dir}/port/esp-idf/h_transport_gpio.c"
    "${host_dir}/port/esp-idf/h_transport_defaults.c"
    "${host_dir}/port/esp-idf/h_transport_common.c"
    "${host_dir}/port/esp-idf/tools/stats.c"
)

# Transport-specific port sources
if(CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE)
    list(APPEND ESP_HOSTED_PORT_SRCS
        "${host_dir}/port/esp-idf/h_transport_sdio.c"
        "${host_dir}/port/esp-idf/h_transport_sdio_bus.c")
elseif(CONFIG_ESP_HOSTED_SPI_HD_HOST_INTERFACE)
    list(APPEND ESP_HOSTED_PORT_SRCS
        "${host_dir}/port/esp-idf/h_transport_spi_hd.c"
        "${host_dir}/port/esp-idf/h_transport_spi_hd_bus.c")
elseif(CONFIG_ESP_HOSTED_SPI_HOST_INTERFACE)
    list(APPEND ESP_HOSTED_PORT_SRCS
        "${host_dir}/port/esp-idf/h_transport_spi.c"
        "${host_dir}/port/esp-idf/h_transport_spi_bus.c")
elseif(CONFIG_ESP_HOSTED_UART_HOST_INTERFACE)
    list(APPEND ESP_HOSTED_PORT_SRCS
        "${host_dir}/port/esp-idf/h_transport_uart.c"
        "${host_dir}/port/esp-idf/h_transport_uart_bus.c")
endif()

# Port-specific private include directories
# IMPORTANT: platform-specific dir MUST come before port/include so that
# h_port_config.h from the port is found before the generic fallback.
set(ESP_HOSTED_PORT_PRIV_INCLUDE_DIRS
    "${host_dir}/port/esp-idf"
    "${host_dir}/port/esp-idf/tools"
    "${host_dir}/port/include"
)

# Port component requirements (ESP-IDF idf_component_register)
# REQUIRES: public dependencies (headers exposed via INCLUDE_DIRS)
set(ESP_HOSTED_PORT_REQUIRES
    esp_wifi
)
# PRIV_REQUIRES: private dependencies (implementation-only)
set(ESP_HOSTED_PORT_PRIV_REQUIRES
    soc esp_netif esp_timer driver esp_wifi bt
    esp_http_client console wpa_supplicant openthread
)
