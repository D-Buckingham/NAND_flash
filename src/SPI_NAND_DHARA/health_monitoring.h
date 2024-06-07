#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>


#include <assert.h>




uint32_t read_bad_block_count(void);
uint32_t read_erase_count(void);
uint32_t read_program_erase_cycles(void);
uint32_t read_ecc_errors(void);

int display_health(void);