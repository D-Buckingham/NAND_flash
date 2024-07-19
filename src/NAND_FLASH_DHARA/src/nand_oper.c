/**
 * @file nand_oper.h
 * @brief Configuring the AS5F14G04SND-10LIN NAND flash
 *
 * This file establishes the communication and stores the predefined commands to interfere 
 * with the 913-S5F14G04SND10LIN NAND flash
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#include <stdint.h>

#include "../inc/nand_oper.h"
#include "../inc/example_handle.h"

/**
 * S5F14G04SND-10LIN
 * 0 ... 4095 blocks RA <17:6>
 * 0 ... 63 pages
 * 0 ... 2175 bytes
 */



#define RA_TO_BLOCK(ra) ((ra >> 6) & 0xFFF)  // Extracts bits 17:6 (block)
#define RA_TO_PAGE(ra)  (ra & 0x3F)         // Extracts bits 5:0 (page)

uint32_t last_read_page_in_NAND_cache = 0;

#ifdef CONFIG_DHARA_METADATA_BUFFER

#define METADATA_SIZE 132


typedef struct {
    uint8_t data[METADATA_SIZE];
    uint32_t page_address;
    uint16_t column;
} metadata_entry_t;

metadata_entry_t metadata_buffer[CONFIG_DHARA_METADATA_BUFFER_SIZE];
uint8_t buffer_index = 0;



// Function to check if the requested metadata is in the buffer
static int find_in_buffer(uint8_t *data, uint32_t page_address, uint16_t column) {
    for (uint8_t i = 0; i < CONFIG_DHARA_METADATA_BUFFER_SIZE; i++) {
        if (metadata_buffer[i].page_address == page_address && metadata_buffer[i].column == column) {
            memcpy(data, metadata_buffer[i].data, METADATA_SIZE);
            return 1; // Found
        }
    }
    return 0; // Not found
}

// Function to store metadata in the buffer
static void store_in_buffer(uint8_t *data, uint32_t page_address, uint16_t column) {
    memcpy(metadata_buffer[buffer_index].data, data, METADATA_SIZE);
    metadata_buffer[buffer_index].page_address = page_address;
    metadata_buffer[buffer_index].column = column;
    buffer_index = (buffer_index + 1) % CONFIG_DHARA_METADATA_BUFFER_SIZE;
}



#endif //CONFIG_DHARA_METADATA_BUFFER

//address_bytes = 0
int nand_write_enable(void)
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

int nand_read_register(uint8_t reg, uint8_t *val)
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

int nand_write_register(uint8_t reg, uint8_t val)
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

int nand_device_id(uint8_t *device_id){

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


int nand_read(uint8_t *data, uint16_t column, uint16_t length) {
    #ifdef CONFIG_DHARA_METADATA_BUFFER
    if (length == METADATA_SIZE && last_read_page_in_NAND_cache != 0) {
        // Check if the metadata is already in the buffer
        if (find_in_buffer(data, last_read_page_in_NAND_cache, column)) {
            return 0; // Data found in buffer, no need to perform read
        }
    }
    #endif //CONFIG_DHARA_METADATA_BUFFER

    nand_transaction_t t = {
        .command = CMD_READ_FAST,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8), // big to small endian
        .miso_len = length, // usually 2 bytes
        .miso_data = data,
        .dummy_bytes = 1
    };

     //my_nand_handle->log("OPER: Reading start at column", false, true, column);
     //my_nand_handle->log("OPER: Reading length", false, true, length);

    if (my_nand_handle && my_nand_handle->transceive) {
        int result = my_nand_handle->transceive(&t);
        #ifdef CONFIG_DHARA_METADATA_BUFFER
        if (result == 0 && length == METADATA_SIZE && last_read_page_in_NAND_cache != 0) {
            // Store the metadata in the buffer
            store_in_buffer(data, last_read_page_in_NAND_cache, column);
        }
        #endif //CONFIG_DHARA_METADATA_BUFFER
        return result;
    } else {
        // Handle error if the function pointer is not set
        if (my_nand_handle && my_nand_handle->log) {
            my_nand_handle->log("Transceive function pointer not set", true, false, 0);
        }
        return -1;
    }
}

int nand_program_load(const uint8_t *data, uint16_t column, uint16_t length)
{
    //last_read_page_in_NAND_cache = 0;
    nand_transaction_t  t = {
        .command = CMD_PROGRAM_LOAD,
        .address_bytes = 2,
        .address = ((column & 0x00FF) << 8) | ((column & 0xFF00) >> 8),
        .mosi_len = length,//(N+1)*8+24
        .mosi_data = data
    };
    //my_nand_handle->log("OPER: Loading start at column", false, true, column);
    //my_nand_handle->log("OPER: Loading length", false, true, length);
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

int nand_read_page(uint32_t page)
{
    //if it already loaded into the NAND cache, don't do it again. (experimental optimization)
    // if(last_read_page_in_NAND_cache == page && page != 0){
    //     return 0;    
    // }
    last_read_page_in_NAND_cache = page;
    nand_transaction_t  t = {
        .command = CMD_PAGE_READ,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };
    //my_nand_handle->log("OPER: Reading page", false, true, page);
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

int nand_program_execute(uint32_t page)
{
    nand_transaction_t  t = {
        .command = CMD_PROGRAM_EXECUTE,
        .address_bytes = 3,
        .address = ((page & 0x00FF0000) >> 16) |  // Move A23-A16 to the correct position (middle byte)
                   ((page & 0x0000FF00))       |  // Keep A15-A8 in its place
                   ((page & 0x000000FF) << 16)   // Move A7-A0 to the top position
    };

    //my_nand_handle->log("OPER: Execution page", false, true, page);
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

int nand_erase_block(uint32_t page)
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




