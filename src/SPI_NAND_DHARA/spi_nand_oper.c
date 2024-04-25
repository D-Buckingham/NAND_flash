/**
 * @file spi_nand_oper.h
 * @brief Configuring the AS5F14G04SND-10LIN NAND flash
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
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>


/**
 * S5F14G04SND-10LIN
 * 0 ... 4095 blocks RA <17:6>
 * 0 ... 63 pages
 * 0 ... 2175 bytes
 */

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE

LOG_MODULE_REGISTER(spi_nand_oper, CONFIG_LOG_DEFAULT_LEVEL);


const struct spi_dt_spec spi_nand_init(void) {
    const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(spidev), SPI_OP, 0);

    if (!device_is_ready((&spidev_dt)->bus)) {
        LOG_ERR("SPI device is not ready");
    } 
    // else {
    //     LOG_INF("NAND flash as SPI device initialized!\n\n");
    // }

    return spidev_dt;
}



/**
 * Executes operation set in struct
 * 0 If successful in master mode.
-errno Negative errno code on failure.
*/
uint8_t dummy_byte_value = 0xFF;
uint8_t var;


int spi_nand_execute_transaction(const struct spi_dt_spec *spidev_dt, spi_nand_transaction_t *transaction)
{
    //TODO write functionalities that write and read
    int ret;
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
        tx_bufs[2].buf = (uint8_t *)transaction -> mosi_data;
        tx_bufs[2].len = transaction -> mosi_len;
        buf_index++;
    }

    //add dummy byte
    if(transaction -> dummy_bytes > 0){
        tx_bufs[3].buf = &dummy_byte_value;//dummy byte
        tx_bufs[3].len = transaction -> dummy_bytes;
        buf_index++;
    }
    const struct spi_buf_set tx = {
		.buffers = tx_bufs,
		.count = buf_index
	};



    
	
    //synchronous
     if(transaction->miso_len == 0){
        ret = spi_write_dt(spidev_dt, &tx);
     }else{
        //receiver preparation
        struct spi_buf rx_bufs[1] = {0};

        
        rx_bufs[0].buf = 
                        ((1 + transaction -> address_bytes + transaction -> mosi_len + transaction -> dummy_bytes) > 1) 
                        ? transaction->miso_data - (1 + transaction -> address_bytes + transaction -> mosi_len + transaction -> dummy_bytes) 
                        : transaction->miso_data;//shifting the pointer
        
        rx_bufs[0].len = transaction->miso_len + 1 + transaction -> address_bytes + transaction -> mosi_len + transaction -> dummy_bytes;//clocking the entire signal

        //rx_bufs[1].buf = NULL;
        //rx_bufs[1].len = 0;

        const struct spi_buf_set rx = {
            .buffers = rx_bufs,
            .count = 1
        };
        
        ret = spi_transceive_dt(spidev_dt, &tx, &rx);
        }
    //ret = spi_transceive_dt(spidev_dt, &tx, &rx);
    return ret;
}


int spi_nand_read_register(const struct spi_dt_spec *dev, uint8_t reg, uint8_t *val)
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

int spi_nand_write_register(const struct spi_dt_spec *dev, uint8_t reg, uint8_t val)
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

int spi_nand_write_enable(const struct spi_dt_spec *dev)
{
    spi_nand_transaction_t  t = {
        .command = CMD_WRITE_ENABLE
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_read_page(const struct spi_dt_spec *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PAGE_READ,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_read(const struct spi_dt_spec *dev, uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_READ_FAST,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8),//big to small endian
        .miso_len = length,//usually 2 bytes
        .miso_data = data,
        .dummy_bytes = 1
    };
    LOG_INF("The read out data length is: %u", length);

    int ret = spi_nand_execute_transaction(dev, &t);
    if (ret != 0) {
        LOG_ERR("SPI NAND transaction failed with error code %d", ret);
        return ret;
    }

    

    // Log the length of the data read
    LOG_INF("Length of data read: %u bytes", length);

    // Log the data read, formatted in rows of 40 bytes
    LOG_INF("Data read from SPI NAND oper:");
    char line_buf[128]; // Buffer to hold one line of output
    int line_idx = 0;   // Index in the current line

    for (size_t i = 0; i < length; i++) {
        line_idx += snprintf(&line_buf[line_idx], sizeof(line_buf) - line_idx, "%02X ", t.miso_data[i]);
        
        if ((i + 1) % 40 == 0 || i == length - 1) {
            LOG_INF("%s", line_buf);  // Print the accumulated line of data
            line_idx = 0;            // Reset index for the next line
        }
    }

    return ret;

    //return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_program_execute(const struct spi_dt_spec *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_EXECUTE,
        .address_bytes = 3,
        .address = page
    };
    LOG_INF("executing loaded page");

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_program_load(const struct spi_dt_spec *dev, const uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_LOAD,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8),//BSWAP_16(column),
        .mosi_len = length,//(N+1)*8+24
        .mosi_data = data
    };
    LOG_INF("The column is: %u", column);
    LOG_INF("The loaded data length is: %u", length);
    //TODO remove
    LOG_INF("Data write on oper lever");
    for (size_t i = 0; i < length; i++) {
        if (i % 40 == 0 && i != 0) {
            LOG_INF("");  // New line every 40 bytes, but not at the start
        }
        printk("%02X ", data[i]);  // Using printk for continuous output on the same line
    }
    if (length % 40 != 0) {
        LOG_INF("");  // Ensure ending on a new line if not already done
    }
    

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_erase_block(const struct spi_dt_spec *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_ERASE_BLOCK,
        .address_bytes = 3,
        .address = page
    };

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_device_id(const struct spi_dt_spec *dev, uint8_t *device_id){
    //ask for device ID

    spi_nand_transaction_t  t = {
        .command = CMD_READ_ID,
        .address_bytes = 1,
        .address = DEVICE_ADDR_READ,
        .miso_len = 1,//command byte + address byte + return buffer
        .miso_data = device_id,
    };

    return spi_nand_execute_transaction(dev, &t);
}


int spi_nand_test(const struct spi_dt_spec *dev){

    LOG_INF("Starting SPI test");

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        //return -ENODEV;
    }

    
    int ret;
    
    uint8_t device_id;
    ret = spi_nand_device_id(dev, &device_id); 
    if (ret != 0) {
        LOG_ERR("Failed to read device ID");
    } else {
        LOG_INF("SPI NAND Device ID: 0x%x ", device_id);
    }
    
    return ret;
}
