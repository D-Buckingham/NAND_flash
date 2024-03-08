/**
 * @file spi_nand_oper.h
 * @brief Configuring the 913-S5F14G04SND10LIN NAND flash
 *
 * 
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#pragma once

#include <zephyr.h>
#include <device.h>
#include <drivers/spi.h>
#include <sys/util.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spi_nand_transaction_t {
    uint8_t command;
    uint8_t address_bytes;
    uint32_t address;
    uint32_t mosi_len;
    const uint8_t *mosi_data;
    uint32_t miso_len;
    uint8_t *miso_data;
    uint32_t dummy_bits;
};

typedef struct spi_nand_transaction_t spi_nand_transaction_t;

#define CMD_WIRTE_DISABLE   0x04
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



#define REG_STATUS          0xC0



#define REG_PROTECT         0xA0 //block lock
#define REG_CONFIG          0xB0 //otp


#define STAT_BUSY           1 << 0
#define STAT_WRITE_ENABLED  1 << 1
#define STAT_ERASE_FAILED   1 << 2
#define STAT_PROGRAM_FAILED 1 << 3
#define STAT_ECC0           1 << 4
#define STAT_ECC1           1 << 5

// Commands, registers, and status flags definitions

int spi_nand_execute_transaction(const struct device *dev, struct spi_config *config, spi_nand_transaction_t *transaction);

int spi_nand_read_register(const struct device *dev, struct spi_config *config, uint8_t reg, uint8_t *val);
int spi_nand_write_register(const struct device *dev, struct spi_config *config, uint8_t reg, uint8_t val);
int spi_nand_write_enable(const struct device *dev, struct spi_config *config);
int spi_nand_read_page(const struct device *dev, struct spi_config *config, uint32_t page);
int spi_nand_read(const struct device *dev, struct spi_config *config, uint8_t *data, uint16_t column, uint16_t length);
int spi_nand_program_execute(const struct device *dev, struct spi_config *config, uint32_t page);
int spi_nand_program_load(const struct device *dev, struct spi_config *config, const uint8_t *data, uint16_t column, uint16_t length);
int spi_nand_erase_block(const struct device *dev, struct spi_config *config, uint32_t page);

#ifdef __cplusplus
}
#endif
