#ifndef SIMULATE_INCOMING_DATA_H
#define SIMULATE_INCOMING_DATA_H

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>



void fill_data_buffer(void);
void write_thread(void);
int simulate_incoming_data(void);

#endif // SIMULATE_INCOMING_DATA_H
