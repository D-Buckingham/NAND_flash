/**
 * @file spi_nand_oper.h
 * @brief Configuring the 913-S5F14G04SND10LIN NAND flash
 *
 * This file establishes the SPI communication and stores the predefined commands to interfere 
 * with the 913-S5F14G04SND10LIN NAND flash
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#include "spi_nand_oper.h"

#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>



LOG_MODULE_REGISTER(spi_nand_oper, CONFIG_LOG_DEFAULT_LEVEL);//TODO maybe adjust level


//Manually create generic SPI struct
#define SPI_CS_PIN 16  // Assuming pin 16 is correct as per &arduino_header definition
#define SPI_CS_FLAGS GPIO_ACTIVE_LOW

#define SPI4_NODE           DT_NODELABEL(arduino_spi)


static const struct spi_config spi_nand_cfg = {
    .frequency = 6000000, // TODO adjust the frequency as necessary
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,//test, should be correct, CPOL = 0, CPHA = 0
    .slave = 0, // SPI slave index
    .cs = {
        .gpio = GPIO_DT_SPEC_GET(SPI4_NODE, cs_gpios),
    },// spi_cs,
};


void spi_nand_init(void){
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(arduino_spi));
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
    }
}


/**
 * Executes operation set in struct
 * 0 If successful in master mode.
-errno Negative errno code on failure.
*/
int spi_nand_execute_transaction(const struct device *dev, spi_nand_transaction_t *transaction)
{
    //TODO write functionalities that write and read
    int ret;

    //transmitter preparation before sending
    struct spi_buf tx_bufs[(transaction->address_bytes) + (transaction->mosi_len) + 1];//address bytes + data bytes + the command byte 
	const struct spi_buf_set tx = {
		.buffers = tx_bufs,
		.count = ARRAY_SIZE(tx_bufs)
	};

	tx_bufs[0].buf = &transaction->command;
	tx_bufs[0].len = 1;
    //add address
    uint8_t addressByte[transaction -> address_bytes];
    for(int cnt = 1;cnt <= transaction -> address_bytes; cnt++){
        int byteIndex = transaction->address_bytes - cnt - 1;
        // Extract the specific byte from the address
        addressByte[cnt] = (transaction->address >> (8 * byteIndex)) & 0xFF;
        tx_bufs[cnt].buf = &addressByte[cnt];
	    tx_bufs[cnt].len = 1;
    }
    //add data
    if(transaction -> mosi_len > 0){//read register uses an additional byte
        tx_bufs[transaction -> address_bytes + 1].buf = &transaction ->mosi_data;
        tx_bufs[transaction -> address_bytes + 1].len = 1;
    }
    //add dummy byte
    if(transaction -> dummy_bytes > 0){
        tx_bufs[transaction -> mosi_len + transaction -> address_bytes + 1].buf = NULL;//dummy byte
        tx_bufs[transaction -> mosi_len + transaction -> address_bytes + 1].len = 1;
    }



    //receiver preparation
    uint8_t buffer_rx[transaction->miso_len];
	struct spi_buf rx_bufs[transaction->miso_len];//from cache only two bytes are read out?
	const struct spi_buf_set rx = {
		.buffers = rx_bufs,
		.count = ARRAY_SIZE(rx_bufs)
	};

    for(int cnt = 0; cnt < transaction->miso_len; cnt++){
        rx_bufs[0].buf = &buffer_rx[cnt];
	    rx_bufs[cnt].len = 1;
    }
	
    //synchronous
    ret = spi_transceive(dev, &spi_nand_cfg, &tx, &rx);

    return ret;
}

int spi_nand_read_register(const struct device *dev, uint8_t reg, uint8_t *val)
{
    spi_nand_transaction_t t = {
        .command = CMD_READ_REGISTER,
        .address_bytes = 1,
        .address = reg,
        .miso_len = 1,
        .miso_data = val
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_write_register(const struct device *dev, uint8_t reg, uint8_t val)
{
    spi_nand_transaction_t  t = {
        .command = CMD_SET_REGISTER,
        .address_bytes = 1,
        .address = reg,
        .mosi_len = 1,
        .mosi_data = &val
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_write_enable(const struct device *dev)
{
    spi_nand_transaction_t  t = {
        .command = CMD_WRITE_ENABLE
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_read_page(const struct device *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PAGE_READ,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_read(const struct device *dev, uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_READ_FAST,
        .address_bytes = 2,
        .address = column,
        .miso_len = length,//usually 2 bytes
        .miso_data = data,
        .dummy_bytes = 8
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_program_execute(const struct device *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_EXECUTE,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_program_load(const struct device *dev, const uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_LOAD,
        .address_bytes = 2,
        .address = column,
        .mosi_len = length,//(N+1)*8+24
        .mosi_data = data
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_erase_block(const struct device *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_ERASE_BLOCK,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_device_id(const struct device *dev, uint8_t *device_id){
    //ask for device ID

    spi_nand_transaction_t  t = {
        .command = CMD_READ_ID,
        .address_bytes = 1,
        .address = DEVICE_ADDR_READ,
        .miso_len = 2,//usually 2 bytes
        .miso_data = device_id,
    };

    return spi_nand_execute_transaction(dev, &t);
}


int spi_nand_test(const struct device *dev){

    LOG_INF("Starting SPI test");

    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        //return -ENODEV;
    }

    uint8_t device_id[2];
    int ret = spi_nand_device_id(dev, device_id);
    if (ret < 0) {
        LOG_ERR("Failed to read device ID");
    } else {
        LOG_INF("SPI NAND Device ID: 0x%x 0x%x", device_id[0], device_id[1]);
    }
    return ret;
}
