# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
#set(DTC_OVERLAY_FILE "boards/arm/nrf5340dk_nrf5340.overlay")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NAND_flash)

file(GLOB APP_SOURCES
            "src/main.c"
            "src/SPI_NAND_DHARA/spi_nand_oper.c"
            "dhara/dhara/*.c"
            "nand_top_layer.c"
            "src/tests/spi_nand_oper_tests.c"
           
)
#FILE(GLOB app_sources src/SPI_NAND_DHARA/spi_nand_oper.c)


include_directories(${PROJECT_SOURCE_DIR}/dhara/dhara)
include_directories(${PROJECT_SOURCE_DIR}/src/SPI_NAND_DHARA)
include_directories(${PROJECT_SOURCE_DIR}/src/tests)

#FILE(GLOB app_sources dhara/dhara/*.c)

#FILE(GLOB app_sources src/SPI_NAND_DHARA/dhara_glue.c)
target_sources(app PRIVATE ${APP_SOURCES})
