
#include "spi_nand_oper.h"


#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


LOG_MODULE_REGISTER(spi_nand, LOG_LEVEL_DBG);//TODO maybe adjust


//Manually create generic SPI struct
#define SPI_CS_PIN 16  // Assuming pin 16 is correct as per &arduino_header definition
#define SPI_CS_FLAGS GPIO_ACTIVE_LOW

static const struct spi_cs_control spi_cs = {
    .gpio_pin = SPI_CS_PIN,
    .gpio_dt_flags = SPI_CS_FLAGS,
    .delay = 3,//typical value from ds, max 4
};

static const struct spi_config spi_nand_cfg = {
    .frequency = 6000000, // TODO adjust the frequency as necessary
    .operation = SPI_WORD_SET(8) | SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_MODE_0,//test, but should be correct
    .slave = 0, // SPI slave index
    .cs = &spi_cs,
};


//might reinitialize them every time as well to keep closer and shorten delays
static struct spi_buf tx_bufs[2];
static const struct spi_buf_set tx = {
    .buffers = tx_bufs,
    .count = ARRAY_SIZE(tx_bufs)
};

static struct spi_buf rx_bufs[2];
static const struct spi_buf_set rx = {
    .buffers = rx_bufs,
    .count = ARRAY_SIZE(rx_bufs)
};



/**
 * Executes operation set in struct
 * TODO create struct and figure out how to encode operations
*/
int spi_nand_execute_transaction(const struct device *dev, struct spi_config *config, spi_nand_transaction_t *transaction)
{
    spi_transaction_ext_t e = {
        .base = {
            .flags = SPI_TRANS_VARIABLE_ADDR |  SPI_TRANS_VARIABLE_CMD |  SPI_TRANS_VARIABLE_DUMMY,//prob not needed
            .rxlength = transaction->miso_len,// * 8
            .rx_buffer = transaction->miso_data,
            .length = transaction->mosi_len,// * 8
            .tx_buffer = transaction->mosi_data,
            .addr = transaction->address,
            .cmd = transaction->command
        },
        .address_bits = transaction->address_bytes //* 8,
        .command_bits = 8,
        .dummy_bits = transaction->dummy_bits
    };

    //TODO write functionalities that write and read
    int ret;

    //transmitter preparation before sending
    struct spi_buf tx_bufs[(transaction->address_bytes) + (transaction->mosi_len) + 1];//address bytes + data bytes + the command byte 
	const struct spi_buf_set tx = {
		.buffers = tx_bufs,
		.count = ARRAY_SIZE(tx_bufs)
	};

	tx_bufs[0].buf = transaction->command;
	tx_bufs[0].len = 8;
    //add address
    for(int cnt = 1;cnt <= transaction -> address_bytes; cnt++){
        int byteIndex = transaction->address_bytes - cnt - 1;
        // Extract the specific byte from the address
        uint8_t addressByte = (transaction->address >> (8 * byteIndex)) & 0xFF;
        tx_bufs[cnt].buf = addressByte;
	    tx_bufs[cnt].len = 8;
    }
    //add data
    if(transaction -> mosi_len > 0){//read register uses an additional byte
        tx_bufs[transaction -> address_bytes + 1].buf = transaction ->mosi_data;
        tx_bufs[transaction -> address_bytes + 1].len = 8;
    }
    //add dummy byte
    if(transaction -> dummy_bytes > 0){
        tx_bufs[transaction -> mosi_len + transaction -> address_bytes + 1].buf = 0x00;//dummy byte
        tx_bufs[transaction -> mosi_len + transaction -> address_bytes + 1].len = 8;
    }



    //receiver preparation

    //load the address, consider cases with different structures


	struct spi_buf rx_bufs[2];
	const struct spi_buf_set rx = {
		.buffers = rx_bufs,
		.count = ARRAY_SIZE(rx_bufs)
	};

	rx_bufs[0].buf = buffer_rx;
	rx_bufs[0].len = BUF_SIZE;

	rx_bufs[1].buf = buffer2_rx;
	rx_bufs[1].len = BUF2_SIZE;

	int ret;

	LOG_INF("Start complete multiple");


    //initalize tx buffer
    uint8_t tx_buffer = ;
    static struct spi_buf tx_bufs[1];
    tx_bufs[0].buf = tx_buffer;
    tx_bufs[0].len = 1;
    static const struct spi_buf_set tx = {
        .buffers = tx_bufs,
        .count = ARRAY_SIZE(tx_bufs)
    };

    static struct spi_buf rx_bufs[2];
    static const struct spi_buf_set rx = {
        .buffers = rx_bufs,
        .count = ARRAY_SIZE(rx_bufs)
    };



    return spi_device_transmit(device, (spi_transaction_t *) &e);
}

int spi_nand_read_register(spi_device_handle_t device, uint8_t reg, uint8_t *val)
{
    spi_nand_transaction_t t = {
        .command = CMD_READ_REGISTER,
        .address_bytes = 1,
        .address = reg,
        .miso_len = 1,
        .miso_data = val
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_write_register(spi_device_handle_t device, uint8_t reg, uint8_t val)
{
    spi_nand_transaction_t  t = {
        .command = CMD_SET_REGISTER,
        .address_bytes = 1,
        .address = reg,
        .mosi_len = 1,
        .mosi_data = &val
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_write_enable(spi_device_handle_t device)
{
    spi_nand_transaction_t  t = {
        .command = CMD_WRITE_ENABLE
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_read_page(spi_device_handle_t device, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PAGE_READ,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_read(spi_device_handle_t device, uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_READ_FAST,
        .address_bytes = 2,
        .address = column,
        .miso_len = length,//usually 2 bytes
        .miso_data = data,
        .dummy_bytes = 8
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_program_execute(spi_device_handle_t device, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_EXECUTE,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_program_load(spi_device_handle_t device, const uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_LOAD,
        .address_bytes = 2,
        .address = column,
        .mosi_len = length,//(N+1)*8+24
        .mosi_data = data
    };

    return spi_nand_execute_transaction(device, &t);
}

esp_err_t spi_nand_erase_block(spi_device_handle_t device, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_ERASE_BLOCK,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(device, &t);
}
