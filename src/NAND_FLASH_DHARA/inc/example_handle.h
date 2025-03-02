#ifndef NAND_HANDLE
#define NAND_HANDLE

#include <stdint.h>
#include "nand_driver.h"
#include "nand_top_layer.h"



// Function to initialize the NAND handle
int init_nand_handle(void);

// Example transceive function


/**
 * @brief Executes a SPI NAND transaction.
 *
 * This function executes a SPI NAND transaction using the currently set transmit function.
 * If no transmit function has been set, a default transmit function will be used.
 *
 * @param transaction Pointer to the SPI NAND transaction structure.
 * @return 0 if successful, or a negative error code on failure.
 */
int my_transceive_function(nand_transaction_t *transaction);

// Example log function
void my_log_function(char *msg, bool is_err, bool has_int_arg, uint32_t arg);

void my_wait_function(uint32_t microseconds);

#endif // NAND_HANDLE
