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


//#define MY_SPI_MASTER DT_NODELABEL(arduino_spi)

#define BUFFER_SIZE 4

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE

#define SPIDEV DT_NODELABEL(arduino_spi)
#define SPI_DEVICE "reg_my_spi_master"



////////////////////////////////////////////////////////////////////////////////////////
///////////////////				TESTING					/////////////////////////////////

uint8_t dummy_byte_value2 = 0xFF;
#define MY_SPI_MASTER DT_NODELABEL(my_spi_master)
#define MY_SPI_MASTER_CS_DT_SPEC SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master))



// SPI master functionality
const struct device *spi_dev;
static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

static void spi_init(void)
{
	spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);
	if(!device_is_ready(spi_dev)) {
		printk("SPI master device not ready!\n");
	}
	struct gpio_dt_spec spim_cs_gpio = MY_SPI_MASTER_CS_DT_SPEC;
	if(!device_is_ready(spim_cs_gpio.port)){
		printk("SPI master chip select device not ready!\n");
	}
}

//| SPI_MODE_CPOL | SPI_MODE_CPHA
static struct spi_config spi_cfg = {
	.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB ,
	.frequency = 4000000,
	.slave = 0,
	.cs = {.gpio = MY_SPI_MASTER_CS_DT_SPEC, .delay = 0},
};



static int spi_nand_execute_transaction2(spi_nand_transaction_t *transaction)
{
    //TODO write functionalities that write and read
    int buf_index = 1;

    struct spi_buf tx_bufs[4] ={0};
    
    //transmitter preparation before sending
    //address bytes + data bytes + the command byte + dummy byte
	

	tx_bufs[0].buf = &transaction->command;
	tx_bufs[0].len = 1;
    
    //add address
    if(transaction -> address_bytes > 0){
        tx_bufs[1].buf = &transaction->address;
        tx_bufs[1].len = transaction -> address_bytes;
        buf_index++;
    }

    //add data
    if(transaction -> mosi_len > 0){
        tx_bufs[2].buf = transaction -> mosi_data;//might expect it not as a pointer
        tx_bufs[2].len = transaction -> mosi_len;
        buf_index++;
    }

    //add dummy byte
    if(transaction -> dummy_bytes > 0){
        tx_bufs[3].buf = &dummy_byte_value2;//dummy byte
        tx_bufs[3].len = transaction -> dummy_bytes;
        buf_index++;
    }
    const struct spi_buf_set tx = {
		.buffers = tx_bufs,
		.count = buf_index
	};

  




    //receiver preparation
	struct spi_buf rx_bufs;//[1] = {0};//from cache only two bytes are read out?

    rx_bufs.buf = transaction->miso_data;
    rx_bufs.len = transaction->miso_len;

    //rx_bufs[1].buf = NULL;
    //rx_bufs[1].len = 0;

    const struct spi_buf_set rx = {
		.buffers = &rx_bufs,
		.count = 1
	};
	

    // Reset signal
    k_poll_signal_reset(&spi_done_sig);


    //synchronous
    int ret = spi_transceive_signal(spi_dev, &spi_cfg, &tx, &rx, &spi_done_sig);
    if(ret != 0){
		printk("SPI transceive error: %i\n", ret);
		return ret;
	}

    // Wait for the done signal to be raised and log the rx buffer
	int spi_signaled, spi_result;
	do{
		k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
	} while(spi_signaled == 0);
    return ret;
}
int spi_nand_device_id2( uint8_t *device_id){
    //ask for device ID

    spi_nand_transaction_t  t = {
        .command = CMD_READ_ID,
        .address_bytes = 1,
        .address = DEVICE_ADDR_READ,
        .miso_len = 4,//usually 2 bytes
        .miso_data = device_id,
        //.dummy_bytes = 1
    };

    return spi_nand_execute_transaction2(&t);
}
int spi_nand_test2(void){

    LOG_INF("Starting SPI test 0");

    

    
    int ret;
    
    uint8_t device_id[4] = {0};
    ret = spi_nand_device_id2(device_id); 
    if (ret != 0) {
        LOG_ERR("Failed to read device ID");
    } else {
        LOG_INF("SPI NAND Device ID 0: 0x%x 0x%x 0x%x 0x%x", device_id[0], device_id[1], device_id[2], device_id[3]);
    }
    
    return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////



int main(void)
{
	LOG_INF("My first breath as an IoT device");
	

	int ret;

	spi_init();

	while(true){
		k_msleep(100);
	
		ret = spi_nand_test2();
	}
	//const struct spi_dt_spec spidev_dt = spi_nand_init();

	//Test the SPI communication
	//spi_nand_test(&spidev_dt);//returns manufacturere and device ID
	//test_SPI_NAND_Communicator_all_tests(&spidev_dt);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl


	//spi_nand_test(dev);//returns manufacturere and device ID
	//test_SPI_NAND_Communicator_all_tests(dev);
	//Test glue between NAND flash communicator and DHARA flash translation layer???

	//test top layer ftl
	

}

