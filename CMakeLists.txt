# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NAND_flash)

file(GLOB APP_SOURCES
            "src/main.c"
            "src/NAND_FLASH_DHARA/src/nand_driver.c"
            "src/NAND_FLASH_DHARA/src/nand_top_layer.c"
            "src/NAND_FLASH_DHARA/dhara/dhara/*.c"
            "src/NAND_FLASH_DHARA/tests/spi_nand_oper_tests.c"
            "src/NAND_FLASH_DHARA/tests/test_spi_nand_top_layer.c"
            "src/NAND_FLASH_DHARA/src/diskio_nand.c"
            "src/NAND_FLASH_DHARA/src/vfs_NAND_flash.c"
            "src/NAND_FLASH_DHARA/tests/test_disk_access.c"
            "src/NAND_FLASH_DHARA/tests/vfs_test.c"
            "src/NAND_FLASH_DHARA/tests/main_nand_tests.c"
            "src/NAND_FLASH_DHARA/src/USB_mass_storage.c"
            "src/NAND_FLASH_DHARA/tests/simulation_test.c"
            "src/NAND_FLASH_DHARA/src/health_monitoring.c"
            "src/NAND_FLASH_DHARA/src/example_handle.c"
            
           
)

include_directories(${PROJECT_SOURCE_DIR}/src/NAND_FLASH_DHARA/dhara/dhara)
include_directories(${PROJECT_SOURCE_DIR}/src/NAND_FLASH_DHARA/inc)
include_directories(${PROJECT_SOURCE_DIR}/src/NAND_FLASH_DHARA/tests)

#FILE(GLOB app_sources dhara/dhara/*.c)

#FILE(GLOB app_sources src/SPI_NAND_DHARA/dhara_glue.c)
target_sources(app PRIVATE ${APP_SOURCES})


# # Ensure Zephyr includes the custom Kconfig
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/NAND_FLASH_DHARA)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/NAND_FLASH_DHARA)

