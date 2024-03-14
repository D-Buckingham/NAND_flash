# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mass)

FILE(GLOB app_sources src/*.c)
FILE(GLOB app_sources src/SPI_NAND_DHARA/spi_nand_oper.c)


include_directories(${PROJECT_SOURCE_DIR}/dhara/dhara)

#FILE(GLOB app_sources dhara/dhara/*.c)

FILE(GLOB app_sources src/SPI_NAND_DHARA/dhara_glue.c)
target_sources(app PRIVATE ${app_sources})
