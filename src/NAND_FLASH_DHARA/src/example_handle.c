#include "../inc/nand_oper.h"
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>


#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE
uint8_t dummy_byte_value = 0xFF;

LOG_MODULE_REGISTER(handle_example, CONFIG_LOG_DEFAULT_LEVEL);


// global handle, include again, keep the name my_nand_handle
nand_h Global_handle;
nand_h *my_nand_handle = &Global_handle;



// Example transceive function
int my_transceive_function(nand_transaction_t *transaction) {
    //transmitter preparation before sending
    //address bytes + data bytes + the command byte + dummy byte
    
	const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(nand_device), SPI_OP, 0);

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


//waiting in non blocking way
// Structure to hold the wait state
struct non_blocking_wait {
    struct k_timer timer;
    bool completed;
};

// Timer expiry function
static void timer_expiry_function(struct k_timer *timer_id) {
    struct non_blocking_wait *wait = CONTAINER_OF(timer_id, struct non_blocking_wait, timer);
    wait->completed = true;
}

// Start a non-blocking wait
static void start_non_blocking_wait(struct non_blocking_wait *wait, uint32_t usec) {
    wait->completed = false;
    k_timer_init(&wait->timer, timer_expiry_function, NULL);
    k_timer_start(&wait->timer, K_USEC(usec), K_NO_WAIT);
}

// Check if the wait is completed
static bool is_non_blocking_wait_completed(struct non_blocking_wait *wait) {
    return wait->completed;
}

//reason for simplifying the function with k_busy_wait is the difference in the timing between the customized timer, the k_sleep and the k_busy_wait. issue #23600
void my_wait_function(uint32_t microseconds){
    if(microseconds < 150){
        k_busy_wait(microseconds);
    }else{
        k_sleep(K_MSEC(1));
        // struct non_blocking_wait wait;
        // start_non_blocking_wait(&wait, microseconds); 

        // while (!is_non_blocking_wait_completed(&wait)) {
        //     // Perform other tasks here while waiting
        //     k_yield(); // Yield to other threads
        // }
    }
}


// Initialization somewhere in your code
int init_nand_handle() {
    // Initialize the handle's function pointers and other members
    my_nand_handle->transceive = my_transceive_function;  
    my_nand_handle->log = my_log_function;
    my_nand_handle->wait = my_wait_function;

    // // Link the input handle to the global handle
    // my_nand_handle = handle;
    return 0;
}

SYS_INIT(init_nand_handle, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);


