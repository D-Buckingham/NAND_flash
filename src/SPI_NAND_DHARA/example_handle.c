#include "spi_nand_oper.h"
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>


#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE
uint8_t dummy_byte_value = 0xFF;

LOG_MODULE_REGISTER(handle_example, CONFIG_LOG_DEFAULT_LEVEL);


// Assume you have a global handle or it's passed around properly
nand_h *my_nand_handle;



// Example transceive function
int my_transceive_function(nand_transaction_t *transaction) {
    //transmitter preparation before sending
    //address bytes + data bytes + the command byte + dummy byte
    
	const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(spidev), SPI_OP, 0);

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

        return spi_write_dt(&spidev_dt, &tx);
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

            return spi_write_dt(&spidev_dt, &tx);
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
            
            return spi_transceive_dt(&spidev_dt, &tx, &rx);
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

        return spi_write_dt(&spidev_dt, &tx);
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

        return spi_write_dt(&spidev_dt, &tx);
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

        return spi_transceive_dt(&spidev_dt, &tx, &rx);
    }
    return 0;
}

// Example log function
void my_log_function(char *msg, bool is_err, bool has_int_arg, uint32_t arg) {
    if (is_err) {
        if (has_int_arg) {
            LOG_ERR("%s: %d", msg, arg);
        } else {
            LOG_ERR("%s", msg);
        }
    } else {
        if (has_int_arg) {
            LOG_INF("%s: %d", msg, arg);
        } else {
            LOG_INF("%s", msg);
        }
    }
}


// Initialization somewhere in your code
int init_nand_handle(nand_h *handle) {
    // Initialize the handle's function pointers and other members
    handle->transceive = my_transceive_function;  // Replace with your actual transceive function
    handle->log = my_log_function;                // Replace with your actual log function if any
    return 0;
}


// const static struct spi_dt_spec spi_nand_init(void) {
//     const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(spidev), SPI_OP, 0);

//     if (!device_is_ready((&spidev_dt)->bus)) {
//         LOG_ERR("SPI device is not ready");
//     }else {
//         LOG_INF("NAND flash as SPI device initialized!");
//     }

//     return spidev_dt;
// }


