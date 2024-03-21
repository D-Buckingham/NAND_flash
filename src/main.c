#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>


#include "SPI_NAND_DHARA/spi_nand_oper.h"
#include "tests/spi_nand_oper_tests.h"

LOG_MODULE_REGISTER(main);


#define MY_SPI_MASTER DT_NODELABEL(arduino_spi)

#define BUFFER_SIZE 4

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE

#define SPIDEV DT_NODELABEL(spi4)

/////////////////////		Try 1		/////////////////////////////////////////////////

// struct spi_cs_control spim_cs = {
// 	.gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master)),
// 	.delay = 0,
// };

// static const struct spi_config spi_cfg = {
// 	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
// 				 SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_LINES_SINGLE,
// 	.frequency = 1000000,
// 	.slave = 0,
// 	.cs =  NULL,// &spim_cs
// };


/////////////////////		Try 1 END		/////////////////////////////////////////////////


/////////////////////		Try 2		/////////////////////////////////////////////////


static struct spi_config spi_cfg = {
    .frequency = 1000000, // TODO adjust the frequency as necessary
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,//test, should be correct, CPOL = 0, CPHA = 0
    .slave = 0, // SPI slave index
    .cs = {
        .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(arduino_spi), cs_gpios),
    },
};




/////////////////////		Try 2 END		/////////////////////////////////////////////////



int main(void)
{
	

	
/////////////////////		Try 3		/////////////////////////////////////////////////
	LOG_INF("My first breath as an IoT device");
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


/////////////////////		Try 4 END		/////////////////////////////////////////////////



/////////////////////		Try 5		/////////////////////////////////////////////////
	///////////////////////////because nothing works, the default example //////////////
	// uint8_t my_buffer = 0xDD;
	// struct spi_buf my_spi_buffer[1];
	// my_spi_buffer[0].buf = &my_buffer;
	// my_spi_buffer[0].len = 1;
	// const struct spi_buf_set tx_buff = { my_spi_buffer, 1 };

/*
	const struct spi_dt_spec mcp3201_dev =
                SPI_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master), SPI_OP, 0);


	while(1){
		int ret = spi_write_dt(&mcp3201_dev, &tx_buff);
		if (ret) { LOG_INF("spi_write status: %d", ret); }
		k_msleep(10);
		
	}

/////////////////////		Try 5 END		/////////////////////////////////////////////////


/////////////////////		Try 6		/////////////////////////////////////////////////

	const struct device *spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);

	uint8_t tx_buffer[BUFFER_SIZE] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};
    struct spi_buf tx_buf = {.buf = tx_buffer, .len = BUFFER_SIZE};
    struct spi_buf rx_buf = {.buf = rx_buffer, .len = BUFFER_SIZE};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
    struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

	int error;
	while(true){
    	//error = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
		error = spi_transceive_dt(&mcp3201_dev, &tx_bufs, &rx_bufs);
		if(error != 0){
			LOG_ERR("SPI transceive error: %i", error);
		}
		
	}
	*/


/////////////////////		Try 6 END		/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	

/////////////////////		Try 1 & 7		/////////////////////////////////////////////////

	// const struct spi_dt_spec spi_dev_dt =
    //             SPI_DT_SPEC_GET(DT_NODELABEL(arduino_spi), SPI_OP, 1);

	// const struct device *spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);

	
	// if (!device_is_ready(&spi_dev)) {
    //     LOG_ERR("Device SPI not ready, aborting test");
    // }
	
	// if(!device_is_ready(spim_cs.gpio.port)){
	// 	LOG_ERR("SPI master chip select device not ready!");
	// }

	/*
	const struct device *spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        LOG_INF("Failed to find SPI device %s\n", SPI_DEV);
        
    }
*/

	// const struct spi_dt_spec spi_dev_dt =
    //             SPI_DT_SPEC_GET(DT_NODELABEL(arduino_spi), SPI_OP, 1);

	struct device *spi_dev_test = DEVICE_DT_GET(DT_NODELABEL(reg_my_spi_master));//DEVICE_DT_GET crashes at built if not found
    if (!spi_dev_test) {
        LOG_INF("Failed to find SPI device %s\n", "arduino_spi");
        
    }
	if (!device_is_ready(spi_dev_test)) {
        LOG_ERR("Device SPI not ready, aborting test");
    }else{
		LOG_INF("Device found");
	}


	uint8_t tx_buffer[BUFFER_SIZE] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};
    struct spi_buf tx_buf = {.buf = tx_buffer, .len = BUFFER_SIZE};
    struct spi_buf rx_buf = {.buf = rx_buffer, .len = BUFFER_SIZE};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
    struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

	int error;
	
	error = spi_transceive(spi_dev_test, &spi_cfg, &tx_bufs, &rx_bufs);
//	error = spi_transceive_dt(&spi_dev_dt, &tx_bufs, &rx_bufs);//, 
	
	
	
    if (error != 0) {
        LOG_INF("SPI transceive failed with error %d\n", error);
    } else {
        LOG_INF("Received: ");
        for (int i = 0; i < BUFFER_SIZE; i++) {
            LOG_INF("0x%X ", rx_buffer[i]);
        }
    }

/////////////////////		Try 7 END		/////////////////////////////////////////////////




	//Test the SPI communication
	//const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(arduino_spi));
	//spi_nand_test(dev);//returns manufacturere and device ID
	//test_SPI_NAND_Communicator_all_tests(dev);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl
	

}

