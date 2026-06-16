# Linux mock port source list
#
# This file mirrors the ESP_HOSTED_PORT_* variables exported by the ESP-IDF
# port and is intended as a reference for future non-ESP ports.
#
# The current Linux mock test path is scripts/run_linux_mock_tests.sh, which
# lists these sources explicitly and builds with the host compiler. Do not
# treat this as a production ESP-IDF-selectable port: the sources depend on
# POSIX APIs and mock transport behavior.

set(ESP_HOSTED_PORT_SRCS
    "${host_dir}/port/linux/src/h_osal.c"
    "${host_dir}/port/linux/src/h_event.c"
    "${host_dir}/port/linux/src/h_transport_mock.c"
    "${host_dir}/port/linux/src/h_wifi.c"
)

# Port-specific private include directories
# IMPORTANT: platform-specific dir MUST come before port/include so that
# h_port_config.h from the port is found before the generic fallback.
set(ESP_HOSTED_PORT_PRIV_INCLUDE_DIRS
    "${host_dir}/port/linux"
    "${host_dir}/port/include"
)

# Port dependency placeholders. A future non-ESP build system may ignore these;
# they are kept to match the port.cmake shape used by the root component.
set(ESP_HOSTED_PORT_REQUIRES "")
set(ESP_HOSTED_PORT_PRIV_REQUIRES "")
