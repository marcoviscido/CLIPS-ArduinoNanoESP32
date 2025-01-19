# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Adds necessary definitions for compiling it using Serial symbol attached to the HW USB CDC port
list(APPEND compile_definitions "ARDUINO_USB_CDC_ON_BOOT=1")
list(APPEND compile_definitions "ARDUINO_USB_MODE=1")
list(APPEND compile_definitions "BOARD_HAS_PSRAM")

list(APPEND compile_definitions "ARDUINO_NANO_ESP32")
# list(APPEND compile_definitions "USB_MANUFACTURER=\\\"Arduino\\\"")
# list(APPEND compile_definitions "USB_PRODUCT=\\\"NanoESP32\\\"")
list(APPEND compile_definitions "ARDUINO_USB_DFU_ON_BOOT=1")
list(APPEND compile_definitions "ARDUINO_RUNNING_CORE=1")
list(APPEND compile_definitions "ARDUINO_EVENT_RUNNING_CORE=1")

project(main)
