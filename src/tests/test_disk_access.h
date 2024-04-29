/**
 * @file test_spi_nand_top_layer.h
 * @brief Testing functions to test the wrapper to the DHARA flash translation layer and the subsequent ones
 *
 * 
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#ifndef TEST_DISK_ACCESS
#define TEST_DISK_ACCESS

#pragma once

#include <zephyr/kernel.h>
#include <Zephyr/device.h>
#include <Zephyr/drivers/spi.h>
#include <Zephyr/sys/util.h>
#include <zephyr/devicetree.h>

#include "diskio_nand.h"



int test_disk_initialize_status_read(const struct disk_info *nand_disk);



#endif //TEST_DISK_ACCESS