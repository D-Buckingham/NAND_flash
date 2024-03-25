#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>

#include <zephyr/devicetree.h>


#include "SPI_NAND_DHARA/spi_nand_oper.h"
#include "tests/spi_nand_oper_tests.h"

LOG_MODULE_REGISTER(main);


#define MY_SPI_MASTER DT_NODELABEL(arduino_spi)

#define BUFFER_SIZE 4

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE

#define SPIDEV DT_NODELABEL(arduino_spi)
#define SPI_DEVICE "reg_my_spi_master"




int main(void)
{
	LOG_INF("My first breath as an IoT device");
	
	const struct spi_dt_spec spidev_dt = spi_nand_init();

	//Test the SPI communication
	spi_nand_test(&spidev_dt);//returns manufacturere and device ID
	test_SPI_NAND_Communicator_all_tests(&spidev_dt);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl


	//spi_nand_test(dev);//returns manufacturere and device ID
	//test_SPI_NAND_Communicator_all_tests(dev);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl
	

}

