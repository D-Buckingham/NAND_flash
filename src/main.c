#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include <zephyr/fs/fs.h>
#include <zephyr/devicetree.h>

#ifndef MAIN


#include "SPI_NAND_DHARA/spi_nand_oper.h"
#include "tests/spi_nand_oper_tests.h"
#include "tests/test_spi_nand_top_layer.h"
#include "vfs_NAND_flash.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


#define MY_SPI_MASTER DT_NODELABEL(arduino_spi)

#define BUFFER_SIZE 4

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE

#define SPIDEV DT_NODELABEL(arduino_spi)
#define SPI_DEVICE "reg_my_spi_master"




int  main(void)
{
	LOG_INF("My first breath as an IoT device");
	
	const struct spi_dt_spec spidev_dt = spi_nand_init();

	//Test the SPI communication
	int ret;
	
	// ret = test_SPI_NAND_Communicator_all_tests(&spidev_dt);
	// if (ret != 0) {
    //     LOG_ERR("Communication tests failed, err: %d", ret);
    // }

	// ret = test_nand_top_layer(&spidev_dt);
	// if (ret != 0) {
    //     LOG_ERR("Top layer DHARA tests failed, err: %d", ret);
    // }

	mount_nand_fs();

	return 0;

}

#endif // MAIN