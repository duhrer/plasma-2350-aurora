cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)

set(AURORA plasma-aurora)

project(${AURORA} C CXX ASM)

# Initialize the SDK
pico_sdk_init()

add_subdirectory(lib/pico_neopixels build)

# Add your source files
add_executable(${AURORA}
    src/aurora.cpp
)

# Link required libraries
target_link_libraries(${AURORA} LINK_PUBLIC
    pico_stdlib
    pico_neopixel
    hardware_pio
    pico_bootsel_via_double_reset
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${AURORA})

# Disable USB serial and enable UART serial
pico_enable_stdio_usb(${AURORA} 0)
pico_enable_stdio_uart(${AURORA} 1)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${AURORA}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
