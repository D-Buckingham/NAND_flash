/**
 * @file nand_driver.h
 * @brief Configuring the 913-S5F14G04SND10LIN NAND flash
 *
 * This file establishes the communication and stores the predefined commands to interfere 
 * with the 913-S5F14G04SND10LIN NAND flash
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#include "nand_top_layer.h"

#ifndef NAND_OPER_H
#define NAND_OPER_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Structure representing a NAND transaction.
 *
 * This structure holds all necessary data for performing a NAND transaction,
 * including command, address, data to send and receive, and dummy bytes for timing purposes.
 * Basically, it gives the structure used to create the order of bytes that need to be sent, independent of
 * communication protocol.
 */
typedef struct {
    uint8_t command;          /**< Command byte to send */
    uint8_t address_bytes;    /**< Number of address bytes */
    uint32_t address;         /**< Address for the transaction */
    uint32_t mosi_len;        /**< Length of the data to send */
    const uint8_t *mosi_data; /**< Pointer to the data to send */
    uint32_t miso_len;        /**< Length of the data to receive */
    uint8_t *miso_data;       /**< Pointer to the buffer for received data */
    uint32_t dummy_bytes;     /**< Number of dummy bytes for timing */
} nand_transaction_t;



//////////////////////////////          Handle DEFINITION          //////////////////////////////////

/**
 * @brief NAND Device Handle struct.
 * Represents a single NAND device. Holds the hardware-specific interface functions,
 * and device configuration.
 */
typedef struct nand_h {
  // === Interface function pointers. Required. ===

  /**
   * @brief Function pointer for transmitting and receiving NAND transactions.
   * @warning Required!
   *
   * @param transaction Pointer to the NAND transaction structure.
   * @return 0 if successful, or a negative error code on failure.
   */
  int (*transceive)(nand_transaction_t *transaction);

  // === Interface function pointers. Optional. ===

  /**
   * @brief Pointer to logging function.
   * Called by the driver to log status and error messages, with an optional integer
   * variable to log. Note that the string does not contain any formatting specifiers,
   * and should be logged as follows (if has_int_arg is true):
   *
   * printf("%s: %s %i", is_err ? "ERR" : "LOG", msg, arg);
   *
   * @param msg the log message
   * @param is_err indicates if this is an error message
   * @param has_int_arg indicates if this message is accompanied by an integer variable to log.
   * @param arg the integer variable to log if has_int_arg is true.
   */
  void (*log)(char *msg, bool is_err, bool has_int_arg, uint32_t arg);


  /**
   * @brief integer representing the flashes that are in parallel
   */
  int number_of_flashes = 1;
  //uint8_t internal_regs[0x76]; //!< For internal use.???
} nand_h;

//Prototype, this handle pointer has to be defined in the example_handle.c
extern nand_h *my_nand_handle;
//////////////////////////////          Handle END          //////////////////////////////////



#define CMD_WRITE_DISABLE   0x04
#define CMD_WRITE_ENABLE    0x06
#define CMD_ERASE_BLOCK     0xD8
#define CMD_PROGRAM_LOAD    0x02
#define CMD_PROGRAM_LOAD_X4 0x32
#define CMD_PROGRAM_EXECUTE 0x10
#define CMD_PROGRAM_LOAD_RAND 0x84
#define CMD_PROGRAM_LOAD_RAND_X4 0xC4
#define CMD_PAGE_READ       0x13
#define CMD_READ_FAST       0x0B
#define CMD_READ_X2         0x3B
#define CMD_READ_X4         0x6B
#define CMD_READ_ID         0x9F

#define CMD_SET_REGISTER    0x1F //commands are used to monitor the device status and alter the device behavior, check this!! TODO
#define CMD_READ_REGISTER   0x0F//The content of status register can be read by issuing the GET FEATURE (0FH) command, followed by the status register address C0H.
#define CMD_Reset           0xFF

#define DEVICE_ADDR_READ    0x01
#define MANUFACTURER_ADDR_READ  0x00



#define REG_STATUS          0xC0
#define REG_PROTECT         0xA0 //block lock
#define REG_CONFIG          0xB0 //otp


#define STAT_BUSY           (1 << 0)
#define STAT_WRITE_ENABLED  (1 << 1)
#define STAT_ERASE_FAILED   (1 << 2)
#define STAT_PROGRAM_FAILED (1 << 3)
#define STAT_ECC0           (1 << 4)
#define STAT_ECC1           (1 << 5)

// Commands, registers, and status flags definitions



/**
 * @brief Read a register from the NAND device.
 *
 * @param reg Register address to read from.
 * @param val Pointer to variable where read value will be stored.
 * @return 0 on success, negative error code otherwise.
 */
int nand_read_register(uint8_t reg, uint8_t *val);

/**
 * @brief Write to a register on the NAND device.
 *
 * @param reg Register address to write to.
 * @param val Value to write to the register.
 * @return 0 on success, negative error code otherwise.
 */
int nand_write_register(uint8_t reg, uint8_t val);

/**
 * @brief Enable writing on the NAND device.
 *
 * This function must be called before any write operation.
 *
 * @return 0 on success, negative error code otherwise.
 */
int nand_write_enable(void);

/**
 * @brief Initiate page read operation to cache.
 *
 * Reads the specified page from NAND to the device's internal cache.
 *
 * @param page Page number to read. 24-bit address consists of 7 dummy bits and 17 page/block address bits
 * @return 0 on success, negative error code otherwise.
 */
int nand_read_page(uint32_t page);

/**
 * @brief Read data from the NAND device's cache.
 *
 * Assumes the data has been previously loaded into cache using
 * nand_read_page().
 *
 * @param dev Device  configuration data obtained from devicetree.
 * @param column Start column (byte) in the page from where to start reading.
 * @param length Number of bytes to read.
 * @return 0 on success, negative error code otherwise.
 */
int nand_read(uint8_t *data, uint16_t column, uint16_t length);

/**
 * @brief Execute a program operation.
 *
 * Commits the data previously loaded into the device's cache to the NAND array.
 *
 * @param page Page number to program.
 * @return 0 on success, negative error code otherwise.
 */
int nand_program_execute(uint32_t page);

/**
 * @brief Load data into the NAND device's cache.
 *
 * @param data Pointer to data to be written to cache.
 * @param column Start column (byte) in the page from where to start writing.
 * @param length Number of bytes to write.
 * @return 0 on success, negative error code otherwise.
 */
int nand_program_load(const uint8_t *data, uint16_t column, uint16_t length);

/**
 * @brief Erase a block on the NAND device.
 *
 * @param page Page number within the block to be erased.
 * @return 0 on success, negative error code otherwise.
 */
int nand_erase_block(uint32_t page);


/**
 * @brief Read out device ID
 * 
 * @param device_id Device ID
*/
int nand_device_id(uint8_t *device_id);




#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif //NAND_OPER_H
