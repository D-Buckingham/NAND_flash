#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>


#include <assert.h>

#include "health_monitoring.h"

//Bad Block Count, percentage of capacity
//Erase Count / Wear Leveling to estimate live cycle, Number of times each block has been erased and programmed
//Program / Erase Cycles, Number of program/erase cycles the flash memory has undergone.
//ECC (Error-Correcting Code) Errors, Indicates the level of data corruption and the effectiveness of error correction. An increasing number of ECC corrections can signal degrading memory cells.



// Structure to hold flash health metrics
struct flash_health_metrics {
    uint32_t bad_block_count;//overall bad block counter
    uint32_t erase_count;//Number of times each block has been erased and programmed
    uint32_t program_erase_cycles;//how many program/erase cycles did the flash memory undergo?
    uint32_t ecc_errors;//Indicates the level of data corruption and the effectiveness of error correction. An increasing number of ECC corrections can signal degrading memory cells
    
};

// Function to initialize and retrieve flash health metrics
void get_flash_health_metrics(struct flash_health_metrics *metrics) {
    // Code to retrieve and populate metrics
    metrics->bad_block_count = read_bad_block_count();
    metrics->erase_count = read_erase_count();
    metrics->program_erase_cycles = read_program_erase_cycles();
    metrics->ecc_errors = read_ecc_errors();
}



int display_health(){
    struct flash_health_metrics metrics;
    get_flash_health_metrics(&metrics);
    LOG_INF("Bad Block Count: %u", metrics.bad_block_count);
    LOG_INF("Erase Count: %u", metrics.erase_count);
    LOG_INF("Program/Erase Cycles: %u", metrics.program_erase_cycles);
    LOG_INF("ECC Errors: %u", metrics.ecc_errors);
}

//checks on first page of each block the total amount of bad blocks out
//spare area 816h 817h
uint32_t read_bad_block_count(void){

    return 0;
}

//counter indicating on how many cycles, first spare area
//spare area 818h 820h
uint32_t read_erase_count(void){
    return 0;
}

//spare area 821h 822h
uint32_t read_program_erase_cycles(void){
    return 0;
}
//spare area 83h 824h
uint32_t read_ecc_errors(void){
    return 0;
}
