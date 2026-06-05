cmake_minimum_required(VERSION 3.16)

# Compile the local esp_hosted component through components/espressif__esp_hosted.
# The wrapper gives the local component the same name as the registry dependency
# required by esp_wifi_remote, avoiding a second registry esp_hosted in the link.

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(host_compile_check)
