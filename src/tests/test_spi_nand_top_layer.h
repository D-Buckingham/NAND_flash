/**
 * @file test_spi_nand_top_layer.h
 * @brief Testing functions to test the wrapper to the DHARA flash translation layer and the subsequent ones
 *
 * 
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#ifndef TEST_SPI_NAND_TOP_LAYER
#define TEST_SPI_NAND_TOP_LAYER

#pragma once

#include <zephyr/kernel.h>
#include <Zephyr/device.h>
#include <Zephyr/drivers/spi.h>
#include <Zephyr/sys/util.h>
#include <zephyr/devicetree.h>

#include "nand_top_layer.h"

int test1_setup_erase_deinit_top_layer(const struct spi_dt_spec *spi);

int test2_writing_tests_top_layer(const struct spi_dt_spec *spi);

int test_nand_top_layer(const struct spi_dt_spec *spidev_dt){

#endif  //TEST_SPI_NAND_TOP_LAYER