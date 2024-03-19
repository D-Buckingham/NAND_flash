#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>


#include "SPI_NAND_DHARA/spi_nand_oper.h"
#include "tests/spi_nand_oper_tests.h"

LOG_MODULE_REGISTER(main);



#define BUFFER_SIZE 4

static const struct spi_config spi_cfg = {
    .frequency = 6000000, // TODO adjust the frequency as necessary
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,//test, should be correct, CPOL = 0, CPHA = 0
    .slave = 0, // SPI slave index
    .cs = {
        .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(arduino_spi), cs_gpios),
    },
};

int main(void)
{

	struct device *spi4nn = device_get_binding("spi4n");
    if (!spi4nn) {
        LOG_INF("Failed to find SPI device %s\n", "spidev");
        
    }

	struct spi_config spi_cfg2 = {
        .frequency = 1625000U,
        .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER,
    };

	const struct device *spi = DEVICE_DT_GET(DT_NODELABEL(spi4));

	if (!device_is_ready(spi)) {
        LOG_ERR("Device SPI not ready, aborting test");
        
    }

	uint8_t tx_buffer[BUFFER_SIZE] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};
    struct spi_buf tx_buf = {.buf = tx_buffer, .len = BUFFER_SIZE};
    struct spi_buf rx_buf = {.buf = rx_buffer, .len = BUFFER_SIZE};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
    struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

    int err = spi_transceive(spi, &spi_cfg2, &tx_bufs, &rx_bufs);


	/*
	const struct device *spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        LOG_INF("Failed to find SPI device %s\n", SPI_DEV);
        
    }
*/
	
    if (err) {
        LOG_INF("SPI transceive failed with error %d\n", err);
    } else {
        LOG_INF("Received: ");
        for (int i = 0; i < BUFFER_SIZE; i++) {
            LOG_INF("0x%X ", rx_buffer[i]);
        }
        LOG_INF("\n");
    }

	LOG_INF("My first breath as an IoT device");
	//Test the SPI communication
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(arduino_spi));
	//spi_nand_test(dev);//returns manufacturere and device ID
	test_SPI_NAND_Communicator_all_tests(dev);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl
	

}

