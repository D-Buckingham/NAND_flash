/**
 * @file spi_nand_oper.h
 * @brief Configuring the AS5F14G04SND-10LIN NAND flash
 *
 * This file establishes the SPI communication and stores the predefined commands to interfere 
 * with the 913-S5F14G04SND10LIN NAND flash
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#include <stdint.h>

#include "spi_nand_oper.h"
#include "example_handle.h"

/**
 * S5F14G04SND-10LIN
 * 0 ... 4095 blocks RA <17:6>
 * 0 ... 63 pages
 * 0 ... 2175 bytes
 */



#define RA_TO_BLOCK(ra) ((ra >> 6) & 0xFFF)  // Extracts bits 17:6 (block)
#define RA_TO_PAGE(ra)  (ra & 0x3F)         // Extracts bits 5:0 (page)



//address_bytes = 0
int spi_nand_write_enable(void)
{
    nand_transaction_t  t = {
        .command = CMD_WRITE_ENABLE
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}



//address_bytes = 1

int spi_nand_read_register(uint8_t reg, uint8_t *val)
{
    nand_transaction_t t = {
        .command = CMD_READ_REGISTER,
        .address_bytes = 1,
        .address = reg,
        .miso_len = 1,
        .miso_data = val
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}

int spi_nand_write_register(uint8_t reg, uint8_t val)
{
    nand_transaction_t  t = {
        .command = CMD_SET_REGISTER,
        .address_bytes = 1,
        .address = reg,
        .mosi_len = 1,
        .mosi_data = &val
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}

int spi_nand_device_id(uint8_t *device_id){

    nand_transaction_t  t = {
        .command = CMD_READ_ID,
        .address_bytes = 1,
        .address = DEVICE_ADDR_READ,
        .miso_len = 1,
        .miso_data = device_id,
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}



//address_bytes = 2

int spi_nand_read(uint8_t *data, uint16_t column, uint16_t length)
{
    nand_transaction_t  t = {
        .command = CMD_READ_FAST,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8),//big to small endian
        .miso_len = length,//usually 2 bytes
        .miso_data = data,
        .dummy_bytes = 1
    };
    

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}

int spi_nand_program_load(const uint8_t *data, uint16_t column, uint16_t length)
{
    nand_transaction_t  t = {
        .command = CMD_PROGRAM_LOAD,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8),
        .mosi_len = length,//(N+1)*8+24
        .mosi_data = data
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}



//address_bytes = 3

int spi_nand_read_page(uint32_t page)
{
    nand_transaction_t  t = {
        .command = CMD_PAGE_READ,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}

int spi_nand_program_execute(uint32_t page)
{
    nand_transaction_t  t = {
        .command = CMD_PROGRAM_EXECUTE,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}

int spi_nand_erase_block(uint32_t page)
{
    nand_transaction_t  t = {
        .command = CMD_ERASE_BLOCK,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    if (my_nand_handle && my_nand_handle->transceive) {
        return my_nand_handle->transceive(&t);
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}




