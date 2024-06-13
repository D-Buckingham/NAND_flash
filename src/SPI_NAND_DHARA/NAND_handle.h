#ifndef SPI_NAND_DRIVER_H
#define SPI_NAND_DRIVER_H

#include <stdint.h>

// Forward declaration of structs (assuming they are declared elsewhere)
struct spi_dt_spec;
typedef struct {
    uint8_t command;
    uint8_t address;
    uint8_t *mosi_data;
    uint8_t *miso_data;
    uint8_t address_bytes;
    uint8_t mosi_len;
    uint8_t miso_len;
} spi_nand_transaction_t;

// Define function pointer type
typedef int (*spi_nand_transmit_fn)(const struct spi_dt_spec *spidev_dt, spi_nand_transaction_t *transaction);

// Structure to hold function pointers for the driver
typedef struct {
    spi_nand_transmit_fn transmit;
    // Add other function pointers if needed
} spi_nand_driver_t;


// // Function prototype for the execute transaction function
// int spi_nand_execute_transaction(const spi_nand_driver_t *driver, const struct spi_dt_spec *spidev_dt, spi_nand_transaction_t *transaction);



#endif // SPI_NAND_DRIVER_H
