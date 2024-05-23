#ifndef SIMULATE_INCOMING_DATA_H
#define SIMULATE_INCOMING_DATA_H

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#define STACK_SIZE 1024
#define PRIORITY 5
#define FILE_NAME "/test_simulation.txt"
#define WRITE_INTERVAL K_SECONDS(1)
#define DATA_SIZE 4096
#define MAX_PATH_LEN 255

extern struct k_thread write_thread_data;
extern struct fs_file_t file;
extern char data[DATA_SIZE];
extern const char *startoffile;

int append_to_file(const char *filename, const char *data, size_t length);
int create_file(const char *fname);
void fill_data_buffer(void);
void write_thread(void);
int lsdir(const char *path);
int delete_file_if_exists(const char *path);
int simulate_incoming_data(void);

#endif // SIMULATE_INCOMING_DATA_H
