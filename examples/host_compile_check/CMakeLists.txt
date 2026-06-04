cmake_minimum_required(VERSION 3.16)

# Compile the local esp_hosted component from this repository root.
# This keeps the check self-contained and avoids requiring a components/
# symlink when the repo is checked out on Windows.
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../..")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(host_compile_check)

