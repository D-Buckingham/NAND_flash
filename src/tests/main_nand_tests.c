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


#define FILE_NAME "sonnet.txt"


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


static void create_and_write_file(const char *filename, const char *data) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return;
    }

    rc = fs_write(&file, data, strlen(data));
    if (rc < 0) {
        printk("Failed to write to file %s: %d\n", filename, rc);
    }

    fs_close(&file);
}

static void read_file(const char *filename) {
    struct fs_file_t file;
    fs_file_t_init(&file);
    char buffer[128];
    int bytes_read;

    int rc = fs_open(&file, filename, FS_O_READ);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return;
    }

    bytes_read = fs_read(&file, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        printk("Failed to read file %s: %d\n", filename, bytes_read);
    } else {
        buffer[bytes_read] = '\0';  // Null-terminate the string
        printk("Read from file %s:\n%s\n", filename, buffer);
    }

    fs_close(&file);
}



static int read_file_out(const char *filename, char *out_buffer, size_t buffer_size) {
    struct fs_file_t file;
    fs_file_t_init(&file);
    int bytes_read;

    int rc = fs_open(&file, filename, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("Failed to open file %s: %d", filename, rc);
        return -1;
    }

    bytes_read = fs_read(&file, out_buffer, buffer_size - 1);
    if (bytes_read < 0) {
        LOG_ERR("Failed to read file %s: %d", filename, bytes_read);
        fs_close(&file);
        return -1;
    }

    // Null-terminate the buffer to make it a valid string
    out_buffer[bytes_read] = '\0';

    LOG_INF("Read from file %s: %s", filename, out_buffer);

    fs_close(&file);

    return bytes_read; // Return the number of bytes read
}


//////////////////////////////////////      END OF STATIC FUNCTIONS     ///////////////////////////////////////

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

    LOG_INF("Overall, Test 2: Creating folder, deleting folder, checking deletion");
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
		LOG_INF("Failed to create directory / Directory already exists");
        rc = fs_unlink(fname1);
		return -1;
	}

    struct fs_dir_t dir;
    struct fs_dirent entry;
    fs_dir_t_init(&dir);

    rc = fs_opendir(&dir, nand_mount_fat.mnt_point);
    if (rc < 0) {
        LOG_ERR("Failed to open directory %s: %d", nand_mount_fat.mnt_point, rc);
        return -1;
    }

    bool found = false;
    while (true) {
        rc = fs_readdir(&dir, &entry);
        if (rc < 0) {
            LOG_ERR("Failed to read directory %s: %d", nand_mount_fat.mnt_point, rc);
            fs_closedir(&dir);
            return -1;
        }

        if (entry.name[0] == '\0') {
            // No more entries
            break;
        }

        if (entry.type == FS_DIR_ENTRY_DIR && strcmp(entry.name, "boot_count") == 0) {
            found = true;
            break;
        }
    }

    fs_closedir(&dir);

    if (!found) {
        LOG_ERR("Directory %s was not found after creation", fname1);
        return -1;
    }

    LOG_INF("Directory %s was successfully created and found", fname1);

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

    //delete the folder
    rc = fs_unlink(fname1);
    if (rc < 0) {
		LOG_INF("Failed to delete directory");
		return -1;
	}
    LOG_INF("Folder deleted");

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
int test_create_file(void) {
    char fname[MAX_PATH_LEN];
    struct fs_statvfs sbuf;
    const char *sonnet = 
        "In Zephyr's realm, where microchips do sing,\n"
        "And circuits dance in symphony's embrace,\n"
        "There dwells a marvel, NAND's enduring king,\n"
        "Whose steadfast cells all data do encase.\n"
        "\n"
        "Within its silicon heart, electrons glide,\n"
        "A maze of gates and channels they traverse,\n"
        "As bits and bytes in ordered lines abide,\n"
        "To store our dreams in memory's universe.\n"
        "\n"
        "From ancient days of core and floppy's spin,\n"
        "To flash's reign, a speed unmatched we see,\n"
        "Yet NAND, with modest grace, the race doth win,\n"
        "Ensuring data's constancy with glee.\n"
        "\n"
        "Oh NAND, thy blocks and pages interlaced,\n"
        "In Zephyr's realm, thy presence is embraced.\n"
        "\n"
        "With every read and write, a tale unfolds,\n"
        "Of knowledge vast, a library untold,\n"
        "From microcosm's depths to space-bound holds,\n"
        "Thy silent work a saga does uphold.\n"
        "\n"
        "Thy cells, like stars in digital array,\n"
        "Illuminate the path for future's dawn,\n"
        "As algorithms weave, and firmwares play,\n"
        "In Zephyr's code, thy legacy is drawn.\n"
        "\n"
        "No tempest fierce nor wear shall thee subdue,\n"
        "For in thy NAND, our trust doth rest secure,\n"
        "With steadfast will, thou doth our data strew,\n"
        "In Zephyr's sky, thy permanence is pure.\n"
        "\n"
        "Oh NAND, thou art the heart of storage might,\n"
        "In Zephyr's hands, thou bring'st our data light.\n"
        "\n"
        "Beneath the surface of thy structure small,\n"
        "A labyrinth of logic subtly laid,\n"
        "Where ones and zeros heed the system's call,\n"
        "And fleeting thoughts in permanence are made.\n"
        "\n"
        "Through cycles wear, thou still dost persevere,\n"
        "An e'er resilient keeper of our lore,\n"
        "In depths of thee, all data holds dear,\n"
        "And future's code in thee shall ever store.\n"
        "\n"
        "As currents flow and voltages do surge,\n"
        "In Zephyr's domain, thy prowess reigns,\n"
        "The bounds of possibility to urge,\n"
        "And break the chains of memory's old chains.\n"
        "\n"
        "Oh NAND, in thee, our digital soul finds rest,\n"
        "In Zephyr's breeze, thou art the silent guest.\n"
        "\n"
        "To realms beyond, where quantum leaps unfold,\n"
        "Thy humble roots in silicon shall last,\n"
        "For in thy cells, the universe is told,\n"
        "A microcosmic echo of the vast.\n"
        "\n"
        "From finite gates to infinity's door,\n"
        "Thou art the bridge o'er which our data treads,\n"
        "In Zephyr's world, thy legacy doth soar,\n"
        "A silent force where innovation spreads.\n"
        "\n"
        "And though the sands of time may wear us all,\n"
        "Thy steadfast form in Zephyr shall remain,\n"
        "A sentinel to answer every call,\n"
        "A guardian of data's boundless plane.\n"
        "\n"
        "Oh NAND, in thee the future is enshrined,\n"
        "In Zephyr's realm, thou art both guide and mind.\n";
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME);

    // Create and write the sonnet into the file
    create_and_write_file(fname, sonnet);

    int rc = lsdir(nand_mount_fat.mnt_point);
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

    char buffer[4096];

    // Read the content back from the file
    int bytes_read = read_file_out(fname, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        LOG_ERR("Failed to read the content of the file %s", fname);
        return -1;
    }

    LOG_INF("Content of file %s:\n%s", fname, buffer);

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

    ret = test_create_file();
    if(ret == 0){
        LOG_INF("Overall Test 3: Creation of file, writing and reading successful!");
    }

    return 0;
}
