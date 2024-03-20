#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>


#include "SPI_NAND_DHARA/spi_nand_oper.h"
#include "tests/spi_nand_oper_tests.h"

LOG_MODULE_REGISTER(main);

/*
#define MY_SPI_MASTER DT_NODELABEL(spi4)

struct spi_cs_control spim_cs = {
	.gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master)),
	.delay = 0,
};

static const struct spi_config spi_cfg = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
				 SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_LINES_SINGLE,
	.frequency = 6000000,
	.slave = 0,
	.cs = (struct spi_cs_control *) &spim_cs,
};

*/
#define BUFFER_SIZE 2

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) 

/*
static const struct spi_config spi_cfg = {
    .frequency = 6000000, // TODO adjust the frequency as necessary
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,//test, should be correct, CPOL = 0, CPHA = 0
    .slave = 0, // SPI slave index
    .cs = {
        .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(arduino_spi), cs_gpios),
    },
};
*/
const struct spi_dt_spec spi_dev =
                SPI_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master), SPI_OP, 1);

int main(void)
{
	/*
	struct device *spi4nn = device_get_binding("spi4n");
    if (!spi4nn) {
        LOG_INF("Failed to find SPI device %s\n", "spidev");
        
    }

	struct spi_config spi_cfg2 = {
        .frequency = 1625000U,
        .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER,
    };
	*/

	//const struct spi_dt_spec spi_dev =
    //            SPI_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master), SPI_OP, 0);

	//const struct device *spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);

	/*
	if (!device_is_ready(&spi_dev)) {
        LOG_ERR("Device SPI not ready, aborting test");
    }
	
	if(!device_is_ready(spim_cs.gpio.port)){
		LOG_ERR("SPI master chip select device not ready!");
	}*/
/*
	uint8_t tx_buffer[BUFFER_SIZE] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};
    struct spi_buf tx_buf = {.buf = tx_buffer, .len = BUFFER_SIZE};
    struct spi_buf rx_buf = {.buf = rx_buffer, .len = BUFFER_SIZE};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
    struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

    
	int error = spi_transceive_dt(&spi_dev, &tx_bufs, &rx_bufs);
	if(error != 0){
		LOG_ERR("SPI transceive error: %i", error);
	}


*/
	static uint8_t tx_buffer[3];
	static uint8_t rx_buffer[3];

	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = sizeof(tx_buffer)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};


	/*
	first 2 bytes of tx specify read/write action and on which register to perform
	last byte of rx contains the respons from the accelerometer
	*/
	tx_buffer[0] = 0xCC;
	tx_buffer[1] = 0xBB;

	int error = spi_transceive_dt(&spi_dev, &tx, &rx);
	if(error != 0){
		LOG_ERR("SPI transceive error: %i\n", error);
		return error;
	}


	

	/*
	const struct device *spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        LOG_INF("Failed to find SPI device %s\n", SPI_DEV);
        
    }
*/
	
    if (error) {
        LOG_INF("SPI transceive failed with error %d\n", error);
    } else {
        LOG_INF("Received: ");
        for (int i = 0; i < BUFFER_SIZE; i++) {
            LOG_INF("0x%X ", rx_buffer[i]);
        }
    }

	LOG_INF("My first breath as an IoT device");
	//Test the SPI communication
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(arduino_spi));
	//spi_nand_test(dev);//returns manufacturere and device ID
	test_SPI_NAND_Communicator_all_tests(dev);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl
	

}

