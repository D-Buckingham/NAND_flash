# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NAND_flash)

file(GLOB APP_SOURCES
            "src/main.c"
            "src/SPI_NAND_DHARA/spi_nand_oper.c"
            "src/SPI_NAND_DHARA/nand_top_layer.c"
            "dhara/dhara/*.c"
            "nand_top_layer.c"
            "src/tests/spi_nand_oper_tests.c"
            "src/tests/test_spi_nand_top_layer.c"
            "src/SPI_NAND_DHARA/diskio_nand.c"
            "src/SPI_NAND_DHARA/vfs_NAND_flash.c"
            "src/tests/test_disk_access.c"
            "src/tests/vfs_test.c"
            "src/tests/main_nand_tests.c"
            "src/SPI_NAND_DHARA/USB_mass_storage.c"
            "src/tests/simulation_test.c"
            "src/SPI_NAND_DHARA/health_monitoring.c"
           
)

include_directories(${PROJECT_SOURCE_DIR}/dhara/dhara)
include_directories(${PROJECT_SOURCE_DIR}/src/SPI_NAND_DHARA)
include_directories(${PROJECT_SOURCE_DIR}/src/tests)

#FILE(GLOB app_sources dhara/dhara/*.c)

#FILE(GLOB app_sources src/SPI_NAND_DHARA/dhara_glue.c)
target_sources(app PRIVATE ${APP_SOURCES})
