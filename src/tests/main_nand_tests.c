#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <assert.h>

#include    "test_spi_nand_top_layer.h"
#include    "spi_nand_oper_tests.h"
#include    "nand_top_layer.h"
#include    "../SPI_NAND_DHARA/vfs_NAND_flash.h"
#include    "../SPI_NAND_DHARA/diskio_nand.h"
#include    "../SPI_NAND_DHARA/spi_nand_oper.h"
#include    "../SPI_NAND_DHARA/nand_top_layer.h"

#define MAX_PATH_LEN 255
#define TEST_FILE_SIZE 547


LOG_MODULE_REGISTER(test_main_top, CONFIG_LOG_DEFAULT_LEVEL);

static int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		LOG_ERR("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	LOG_PRINTK("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			if (res < 0) {
				LOG_ERR("Error reading dir [%d]\n", res);
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

	return res;
}

//is a device connected?
//get device through dhara and check if it is there
int top_device_connected(void){
    int res;
    struct fs_statvfs sbuf;
    uint32_t sector_size;

    LOG_INF("Overall, Test 1: Checking initialization on every level");

    //device connected?
    const struct spi_dt_spec spidev_dt = spi_nand_init();//gets pointer to device from DT
    if (!device_is_ready(spidev_dt.bus)) {
        LOG_ERR("Device not ready on SPI level");
        return -1;
    }else{
        LOG_INF("Device correctly retrieved from device tree");
    }


    //check if the communication works by reading out the device ID
    res = test_IDs_spi_nand(&spidev_dt);
    if (res != 0) {
        LOG_ERR("Device & Manufacturer ID test failed");
        return -1;
    }


    //get sector size to validate if dhara map layer works
    //nand_flash_config.spi_dev = &spidev_dt; //unnecessary

    //spi_nand_flash_init_device(&nand_flash_config, &device_handle); // already initialized
    res = spi_nand_flash_get_sector_size(device_handle, &sector_size);
    if(res != 0){
        LOG_ERR("Unable to get sector size, error: %d", res);
        return -1;
    }  else{
        LOG_INF("Size of sector is: %u", sector_size);
    }


    //disk properly connected?
    if(nand_disk_access_status(&nand_disk) != DISK_STATUS_OK){
        LOG_ERR("Failed to check status!");
        return -1;
    }else{
        LOG_INF("Disk access status = ok");
    }


    //if flash correctly mounted
    res = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (res < 0) {
		LOG_ERR("FAIL: statvfs: %d\n", res);
        return -1;
	}else{
    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);
    }

    
    return 0;
}


/**
 * @brief Test if a folder can be created on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_create_folder(void){
    struct fs_statvfs sbuf;
    int rc;
    char fname1[MAX_PATH_LEN];
    snprintf(fname1, sizeof(fname1), "%s/boot_count", nand_mount_fat.mnt_point);

    //Print out current state
    LOG_INF("Directories before adding a folder");
    rc = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (rc < 0) {
		LOG_PRINTK("FAIL: statvfs: %d\n", rc);
		return -1;
	}

	LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

	rc = lsdir(nand_mount_fat.mnt_point);
    if (rc < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
		return -1;
	}

    //add a folder
    rc = fs_mkdir(fname1);
    if (rc < 0) {
		LOG_INF("Failed to create directory");
		return -1;
	}

    LOG_INF("Directories after adding a folder");
    rc = lsdir(nand_mount_fat.mnt_point);
    if (rc < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
		return -1;
	}
    rc = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (rc < 0) {
		LOG_PRINTK("FAIL: statvfs: %d\n", rc);
		return -1;
	}

	LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

    return 0;
}

/**
 * @brief Test if a file can be created on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_create_file(void){
    return 0;
}

/**
 * @brief Test if a file can be read from the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_read_file(void){
    return 0;
}

/**
 * @brief Test storing a large file that spans multiple blocks on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_store_large_file(void){
    return 0;
}

/**
 * @brief Test appending data to a large file and inspect the changes.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_append_data_large_file(void){
    return 0;
}

/**
 * @brief Test changing the data of a file and analyze the process on the NAND device.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_change_file_data(void){
    return 0;
}

/**
 * @brief Test deleting a file from the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_delete_file(void){
    return 0;
}

/**
 * @brief Test writing to one-eighth of the flash memory.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_write_one_eighth_flash(void){
    return 0;
}

/**
 * @brief Test how multiple small files are stored in the Dhara mapping layer.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_store_multiple_small_files(void){
    return 0;
}


//Can I create a folder?


//Can I create a file?


//Can I read the file?


//How to store a big file? Stored over multiple blocks?


//Adding data to big file, how is it changed on the actual device? Inspect mapping of dhara


//Change data of a file, how is this process handled in dhara/ on the actual device?


//How is a file deleted?


//Write to a 1/8 of the flash, how is it done?


//How are multiple small files stored in dhara?



//Main function in which the other ones are called
int test_all_main_nand_tests(void){

    //is the device on each layer connected/initialized?
    int ret = top_device_connected();
    if(ret == 0){
        LOG_INF("Overall Test 1: All modalities sucessful recognized!");
    }

    ret = test_create_folder();
    if(ret == 0){
        LOG_INF("Overall Test 2: Creation of folder successful!");
    }

    return 0;
}
