#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>


#include "SPI_NAND_DHARA/spi_nand_oper.h"

LOG_MODULE_REGISTER(main);




int main(void)
{

	LOG_INF("My first breath as an IoT device");
	//Test the SPI communication
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(arduino_spi));
	spi_nand_test(dev);

}

