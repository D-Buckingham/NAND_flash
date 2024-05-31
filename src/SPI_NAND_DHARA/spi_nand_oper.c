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

#define RA_TO_BLOCK(ra) ((ra >> 6) & 0xFFF)  // Extracts bits 17:6 (block)
#define RA_TO_PAGE(ra)  (ra & 0x3F)         // Extracts bits 5:0 (page)


LOG_MODULE_REGISTER(spi_nand_oper, LOG_LEVEL_INF);

uint8_t dummy_byte_value = 0xFF;
uint8_t var;


const struct spi_dt_spec spi_nand_init(void) {
    const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(spidev), SPI_OP, 0);

    if (!device_is_ready((&spidev_dt)->bus)) {
        LOG_ERR("SPI device is not ready");
    }else {
        LOG_INF("NAND flash as SPI device initialized!");
    }

    return spidev_dt;
}



/**
 * Executes operation set in struct
 * 0 If successful in master mode.
-errno Negative errno code on failure.
*/


int spi_nand_execute_transaction(const struct spi_dt_spec *spidev_dt, spi_nand_transaction_t *transaction)
{
    //transmitter preparation before sending
    //address bytes + data bytes + the command byte + dummy byte
	

    //handle transmissions of 1 byte 
    //(CMD_WRITE_ENABLE and CMD_WRITE_DISABLE)
    if (transaction->address_bytes == 0) {
        struct spi_buf tx_bufs_one_byte;
        tx_bufs_one_byte.buf = &transaction->command;
        tx_bufs_one_byte.len = 1;

        const struct spi_buf_set tx = {
            .buffers = &tx_bufs_one_byte,
            .count = 1
        };

        return spi_write_dt(spidev_dt, &tx);
    }




    //handle transmissions of 3 bytes
    if (transaction->address_bytes == 1) {

        //differentiate between only writing and or transceiving

        //send: set feature
        if(transaction->miso_len == 0){
            uint8_t combined_buf[3]; 

            combined_buf[0] = transaction->command;
            combined_buf[1] = transaction->address;
            combined_buf[2] = *(uint8_t *)transaction->mosi_data;//it's only 1 byte

            struct spi_buf tx_bufs_three_bytes;
            tx_bufs_three_bytes.buf = combined_buf;
            tx_bufs_three_bytes.len = 3;

            const struct spi_buf_set tx = {
                .buffers = &tx_bufs_three_bytes,
                .count = 1
            };

            return spi_write_dt(spidev_dt, &tx);
        }else{
            uint8_t combined_buf[2];
            combined_buf[0] = transaction->command;
            combined_buf[1] = transaction->address;

            struct spi_buf tx_bufs_two_bytes;
            tx_bufs_two_bytes.buf = combined_buf;
            tx_bufs_two_bytes.len = 2;

            const struct spi_buf_set tx = {
                .buffers = &tx_bufs_two_bytes,
                .count = 1
            };

            struct spi_buf rx_bufs;

            rx_bufs.buf = transaction->miso_data - 2;//shifting the pointer      
            rx_bufs.len = 3;

            const struct spi_buf_set rx = {
                .buffers = &rx_bufs,
                .count = 1
            };
            
            return spi_transceive_dt(spidev_dt, &tx, &rx);
        }
    }



    //handle transmissions of 4 bytes / address bytes = 3
    if (transaction->address_bytes == 3){//block erase, program execute, page read to cache
        uint8_t combined_buf[4]; 
        uint8_t *address_bytes = (uint8_t *)&transaction->address;

        combined_buf[0] = transaction->command;
        combined_buf[1] = address_bytes[0]; // A23-A16
        combined_buf[2] = address_bytes[1]; // A15-A8
        combined_buf[3] = address_bytes[2]; // A7-A0

        struct spi_buf tx_bufs_four_bytes;
        tx_bufs_four_bytes.buf = combined_buf;
        tx_bufs_four_bytes.len = 4;

        const struct spi_buf_set tx = {
            .buffers = &tx_bufs_four_bytes,
            .count = 1
        };

        return spi_write_dt(spidev_dt, &tx);
    }

    //transmissions of more than 4 bytes/ program load
    if(transaction->mosi_len > 0){
        uint8_t combined_buf[3]; 
        uint8_t *address_bytes = (uint8_t *)&transaction->address;

        combined_buf[0] = transaction->command;
        combined_buf[1] = address_bytes[0]; // A23-A16
        combined_buf[2] = address_bytes[1]; // A15-A8
        

        struct spi_buf tx_bufs_many_bytes[2] = {0};
        tx_bufs_many_bytes[0].buf = combined_buf;
        tx_bufs_many_bytes[0].len = 3;

        tx_bufs_many_bytes[1].buf = (uint8_t *)transaction -> mosi_data;
        tx_bufs_many_bytes[1].len = transaction -> mosi_len;

        const struct spi_buf_set tx = {
            .buffers = tx_bufs_many_bytes,
            .count = 2
        };

        return spi_write_dt(spidev_dt, &tx);
    }

    if(transaction->miso_len > 0){ //read from cache
        uint8_t combined_buf[4];
        uint8_t *address_bytes = (uint8_t *)&transaction->address;
        combined_buf[0] = transaction->command;
        combined_buf[1] = address_bytes[0]; // A23-A16
        combined_buf[2] = address_bytes[1]; // A15-A8
        combined_buf[3] = dummy_byte_value;

        struct spi_buf tx_bufs_four_bytes_miso;
        tx_bufs_four_bytes_miso.buf = combined_buf;
        tx_bufs_four_bytes_miso.len = 4;

        const struct spi_buf_set tx = {
            .buffers = &tx_bufs_four_bytes_miso,
            .count = 1
        };

        struct spi_buf rx_bufs;

        rx_bufs.buf = transaction->miso_data - 4;//shifting the pointer      
        rx_bufs.len = transaction->miso_len + 4;

        const struct spi_buf_set rx = {
            .buffers = &rx_bufs,
            .count = 1
        };

        return spi_transceive_dt(spidev_dt, &tx, &rx);
    }
    return 0;
}



//address_bytes = 0
int spi_nand_write_enable(const struct spi_dt_spec *dev)
{
    spi_nand_transaction_t  t = {
        .command = CMD_WRITE_ENABLE
    };

    return spi_nand_execute_transaction(dev, &t);
}



//address_bytes = 1

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

int spi_nand_device_id(const struct spi_dt_spec *dev, uint8_t *device_id){

    spi_nand_transaction_t  t = {
        .command = CMD_READ_ID,
        .address_bytes = 1,
        .address = DEVICE_ADDR_READ,
        .miso_len = 1,
        .miso_data = device_id,
    };

    return spi_nand_execute_transaction(dev, &t);
}



//address_bytes = 2

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
    

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_program_load(const struct spi_dt_spec *dev, const uint8_t *data, uint16_t column, uint16_t length)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_LOAD,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8),
        .mosi_len = length,//(N+1)*8+24
        .mosi_data = data
    };

    //LOG_INF("OPER LEVEL: length of written data %d", length);
    return spi_nand_execute_transaction(dev, &t);
}



//address_bytes = 3

int spi_nand_read_page(const struct spi_dt_spec *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PAGE_READ,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    // uint32_t block = RA_TO_BLOCK(page);
    // uint32_t page2 = RA_TO_PAGE(page);
    // LOG_INF("OPER LEVEL: Reading from Block: %d, Page: %d", block, page2);


    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_program_execute(const struct spi_dt_spec *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_PROGRAM_EXECUTE,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    // uint32_t block = RA_TO_BLOCK(page);
    // uint32_t page2 = RA_TO_PAGE(page);
    // LOG_INF("OPER LEVEL: Execute to NAND array, Block: %d, Page: %d", block, page2);

    return spi_nand_execute_transaction(dev, &t);
}

int spi_nand_erase_block(const struct spi_dt_spec *dev, uint32_t page)
{
    spi_nand_transaction_t  t = {
        .command = CMD_ERASE_BLOCK,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    // uint32_t block = RA_TO_BLOCK(page);
    // uint32_t page2 = RA_TO_PAGE(page);
    // LOG_INF("OPER LEVEL: Erasing Block: %d, Page: %d", block, page2);

    return spi_nand_execute_transaction(dev, &t);
}




