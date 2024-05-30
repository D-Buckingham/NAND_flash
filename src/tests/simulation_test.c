#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <assert.h>

#include "test_spi_nand_top_layer.h"
#include "spi_nand_oper_tests.h"
#include "nand_top_layer.h"
#include "../SPI_NAND_DHARA/vfs_NAND_flash.h"
#include "../SPI_NAND_DHARA/diskio_nand.h"
#include "../SPI_NAND_DHARA/spi_nand_oper.h"
#include "../SPI_NAND_DHARA/nand_top_layer.h"

#include "simulation_test.h"

#define STACK_SIZE 2048
#define PRIORITY 5
#define FILE_NAME "/test_simulation.txt"
#define WRITE_INTERVAL K_SECONDS(1)
#define DATA_SIZE 2047
#define MAX_PATH_LEN 255

K_THREAD_STACK_DEFINE(write_stack_area, STACK_SIZE);
struct k_thread write_thread_data;

static struct fs_file_t file;
static char data[DATA_SIZE] = {0};  // 2KB buffer
const char *startoffile = "This is a header\n";

LOG_MODULE_REGISTER(simulation_test, LOG_LEVEL_INF);

static int append_to_file(const char *filename, const char *data, size_t length) {
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_RDWR | FS_O_APPEND);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    rc = fs_write(&file, data, length);
    if (rc < 0) {
        printk("Failed to write to file %s: %d\n", filename, rc);
        fs_close(&file);
        return -1;
    }
    fs_close(&file);
    return 0;
}

/**
 * @brief Test if a file can be created on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int create_file(const char *fname) {
    LOG_INF("Creating file test_simulation.txt");

    fs_file_t_init(&file);

    int rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", fname, rc);
        return -1;
    }

    size_t length = strlen(startoffile);
    rc = fs_write(&file, startoffile, length);
    if (rc < 0) {
        printk("Failed to write to file %s: %d\n", fname, rc);
        fs_close(&file);
        return -1;
    }

    fs_close(&file);   
    return 0;
}

void fill_data_buffer(void) {
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = '0' + (i % 10);  // Fill buffer with characters '0' to '9'
    }
}

void write_thread(void) {
    char fname[MAX_PATH_LEN];
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME);

    while (1) {
        append_to_file(fname, data, DATA_SIZE);
        k_sleep(WRITE_INTERVAL);
    }
}

static int lsdir(const char *path)
{
	int ret;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	ret = fs_opendir(&dirp, path);
	if (ret) {
		LOG_ERR("Error opening dir %s [%d]\n", path, ret);
		return ret;
	}

	LOG_PRINTK("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		ret = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (ret || entry.name[0] == 0) {
			if (ret < 0) {
				LOG_ERR("Error reading dir [%d]\n", ret);
			}
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			LOG_PRINTK("[DIR ] %s\n", entry.name);
		} else {
			LOG_PRINTK("[FILE] %s (size = %zu)\n",
				   entry.name, entry.size);
		}
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);

	return ret;
}

static int delete_file_if_exists(const char *path) {
    struct fs_dirent entry;
    int ret = fs_stat(path, &entry);

    if (ret == 0 && entry.type == FS_DIR_ENTRY_FILE) {
        ret = fs_unlink(path);
        if (ret < 0) {
            printk("Failed to delete file %s: %d\n", path, ret);
            return -1;
        }
        printk("File %s deleted successfully\n", path);
    }

    return ret;
}


int simulate_incoming_data(void) {
    int ret;
    struct fs_statvfs sbuf;
    char fname[MAX_PATH_LEN];

    ret = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (ret < 0) {
		LOG_ERR("FAIL: statvfs: %d\n", ret);
        return -1;
	}else{
    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);
    }
    ret = lsdir(nand_mount_fat.mnt_point);
    if (ret < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, ret);
		return -1;
	}

    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME);
    delete_file_if_exists(fname);
    

    
    ret = create_file(fname);

    fill_data_buffer();

    ret = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (ret < 0) {
		LOG_ERR("FAIL: statvfs: %d\n", ret);
        return -1;
	}else{
    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);
    }
    ret = lsdir(nand_mount_fat.mnt_point);
    if (ret < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, ret);
		return -1;
	}

    LOG_INF("Starting virtual sampling");

    k_thread_create(&write_thread_data, write_stack_area, K_THREAD_STACK_SIZEOF(write_stack_area),
                    (k_thread_entry_t)write_thread, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);
    return 0;
}
