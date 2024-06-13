#include "spi_nand_driver.h"
#include <zephyr.h>

int custom_spi_nand_transmit(const struct spi_dt_spec *spidev_dt, spi_nand_transaction_t *transaction) {
    // Custom implementation
    return 0; // return appropriate value
}

int main() {
    struct spi_dt_spec spidev_dt = spi_nand_init();
    spi_nand_transaction_t transaction;

    // Set the custom transmit function, if not set, it uses the default
    spi_nand_set_transmit_function(custom_spi_nand_transmit);

    
    return 0;
}
