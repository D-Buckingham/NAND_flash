#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>


#include <assert.h>

#include    "test_spi_nand_top_layer.h"
#include    "spi_nand_oper_tests.h"
#include    "nand_top_layer.h"
#include    "vfs_NAND_flash.h"
#include    "diskio_nand.h"
#include    "nand_oper.h"
#include    "nand_top_layer.h"

#define MAX_PATH_LEN 255
#define TEST_FILE_SIZE 547
#define FILE_NAME "sonnet.txt"
#define FILE_NAME_LARGE "large_file.txt"
#define FILE_NAME_ONE_EIGHTH "one_eigth_file.txt"
#define FILE_CONTENT "This is a test content for a large file. "
#define APPEND_CONTENT "Appending this new data to the large file. "
#define CONTENT_REPEAT_COUNT 1500 // Adjust this to make the file large enough
#define READ_CHUNK_SIZE 2048
#define WRITE_CHUNK_SIZE 2048

#define PATTERN_SEED    0x12345678
LOG_MODULE_REGISTER(test_main_top, CONFIG_LOG_DEFAULT_LEVEL);

/////////////////////////////////////////////       FUNCTIONALITY TESTS START     //////////////////////////

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE
const struct spi_dt_spec nand_init(void) {
    const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(spidev), SPI_OP, 0);

    if (!device_is_ready((&spidev_dt)->bus)) {
        LOG_ERR("SPI device is not ready");
    }else {
        LOG_INF("NAND flash as SPI device initialized!");
    }

    return spidev_dt;
}

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


static int check_and_delete_corrupted_files(const char *dir_path) {
    struct fs_dir_t dir;
    struct fs_dirent entry;
    fs_dir_t_init(&dir);

    int rc = fs_opendir(&dir, dir_path);
    if (rc < 0) {
        LOG_ERR("Failed to open directory %s: %d", dir_path, rc);
        return -1;
    }

    while (true) {
        rc = fs_readdir(&dir, &entry);
        if (rc < 0) {
            LOG_ERR("Failed to read directory %s: %d", dir_path, rc);
            fs_closedir(&dir);
            return -1;
        }
        if (entry.name[0] == '\0') {
            break; // End of directory
        }

        char filepath[MAX_PATH_LEN];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry.name);

        if (entry.type == FS_DIR_ENTRY_FILE) {
            struct fs_file_t file;
            fs_file_t_init(&file);
            int rc = fs_open(&file, filepath, FS_O_READ);
            if (rc < 0) {
                LOG_ERR("Failed to open file %s: %d", filepath, rc);
                int ret = fs_unlink(filepath);
                if (ret < 0) {
                    LOG_ERR("Failed to delete file %s: %d", filepath, ret);
                    fs_closedir(&dir);
                    return -1;
                }
                LOG_INF("File %s deleted successfully", filepath); // Delete if unable to open, assuming corruption
            } else {
                fs_close(&file);
            }
        } else if (entry.type == FS_DIR_ENTRY_DIR) {
            // Recursively check the subdirectory
            check_and_delete_corrupted_files(filepath);
        }
    }

    fs_closedir(&dir);
    return 0;
}


// void delete_all_files(const char *path)
// {
//     struct fs_dir_t dir;
//     struct fs_dirent entry;
//     int ret;

//     fs_dir_t_init(&dir);
//     ret = fs_opendir(&dir, path);
//     if (ret) {
//         LOG_ERR("Error opening directory %s: %d", path, ret);
//         return;
//     }

//     while (true) {
//         ret = fs_readdir(&dir, &entry);
//         if (ret) {
//             LOG_ERR("Error reading directory %s: %d", path, ret);
//             break;
//         }

//         // If the name is empty, we've reached the end of the directory
//         if (entry.name[0] == '\0') {
//             break;
//         }

//         // Construct the full path to the entry
//         char full_path[MAX_PATH_LEN];
//         snprintf(full_path, sizeof(full_path), "%s/%s", path, entry.name);

//         if (entry.type == FS_DIR_ENTRY_DIR) {
//             // Recursively delete files in the subdirectory
//             delete_all_files(full_path);

//             // Delete the directory itself
//             ret = fs_unlink(full_path);
//             if (ret) {
//                 LOG_ERR("Error deleting directory %s: %d", full_path, ret);
//             } else {
//                 LOG_INF("Deleted directory: %s", full_path);
//             }
//         } else {
//             // Delete the file
//             ret = fs_unlink(full_path);
//             if (ret) {
//                 LOG_ERR("Error deleting file %s: %d", full_path, ret);
//             } else {
//                 LOG_INF("Deleted file: %s", full_path);
//             }
//         }
//     }

//     ret = fs_closedir(&dir);
//     if (ret) {
//         LOG_ERR("Error closing directory %s: %d", path, ret);
//     }
// }


#define WRITE_CHUNK_SIZE2 2048 // Define the chunk size as needed

static int create_and_write_file_in_chunks_speed(const char *filename, size_t total_size) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t total_bytes_written = 0;
    size_t content_len = strlen(FILE_CONTENT);

    while (total_bytes_written < total_size) {
        size_t bytes_to_write = MIN(WRITE_CHUNK_SIZE2, total_size - total_bytes_written);
        size_t bytes_written_this_time = 0;

        while (bytes_written_this_time < bytes_to_write) {
            size_t remaining_bytes_to_write = MIN(content_len, bytes_to_write - bytes_written_this_time);
            rc = fs_write(&file, FILE_CONTENT, remaining_bytes_to_write);
            if (rc < 0) {
                printk("Failed to write to file %s: %d\n", filename, rc);
                fs_close(&file);
                return -1;
            }
            bytes_written_this_time += remaining_bytes_to_write;
        }

        total_bytes_written += bytes_to_write;
    }

    fs_close(&file);
    return 0;
}

#define READ_CHUNK_SIZE2 2048 // Define the chunk size as needed

static int read_file_in_chunks(const char *filename, size_t total_size) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_READ);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t total_bytes_read = 0;
    char buffer[READ_CHUNK_SIZE2];

    while (total_bytes_read < total_size) {
        size_t bytes_to_read = MIN(READ_CHUNK_SIZE2, total_size - total_bytes_read);
        rc = fs_read(&file, buffer, bytes_to_read);
        if (rc < 0) {
            printk("Failed to read from file %s: %d\n", filename, rc);
            fs_close(&file);
            return -1;
        }
        total_bytes_read += bytes_to_read;
    }

    fs_close(&file);
    return total_bytes_read; // Return the number of bytes read
}



static int create_and_write_file(const char *filename, const char *data) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t length = strlen(data);
    rc = fs_write(&file, data, length);
    if (rc < 0) {
        printk("Failed to write to file %s: %d\n", filename, rc);
        fs_close(&file);
        return -1;
    }

    // Ensure data is flushed to the file
    // rc = fs_sync(&file);
    // if (rc < 0) {
    //     LOG_ERR("Failed to sync file %s: %d", filename, rc);
    //     return -1;
    // }
    fs_close(&file);
    

    return 0;
}


static int create_and_write_file_without_sync(const char *filename, const char *data, size_t length) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR);
    
    size_t total_bytes_written = 0;

    while (total_bytes_written < length) {
        size_t bytes_to_write = MIN(WRITE_CHUNK_SIZE, length - total_bytes_written);
        rc = fs_write(&file, data + total_bytes_written, bytes_to_write);
        if (rc < 0) {
            fs_close(&file);
            return -1;
        }
        total_bytes_written += bytes_to_write;
    }
        
    fs_close(&file);
    return 0;
}

// static void read_file(const char *filename) {
//     struct fs_file_t file;
//     fs_file_t_init(&file);
//     char buffer[4096];
//     int bytes_read;

//     int rc = fs_open(&file, filename, FS_O_READ);
//     if (rc < 0) {
//         printk("Failed to open file %s: %d\n", filename, rc);
//         return;
//     }

//     bytes_read = fs_read(&file, buffer, sizeof(buffer) - 1);
//     if (bytes_read < 0) {
//         printk("Failed to read file %s: %d\n", filename, bytes_read);
//     } else {
//         buffer[bytes_read] = '\0';  // Null-terminate the string
//         printk("Read from file %s:\n%s\n", filename, buffer);
//     }

//     fs_close(&file);
// }



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


//#define READ_CHUNK_SIZE 1024 // Define the chunk size as needed
static int read_file_out_without_logging(const char *filename, char *out_buffer, size_t buffer_size) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_READ);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t total_bytes_read = 0;
    while (total_bytes_read < buffer_size) {
        size_t bytes_to_read = MIN(READ_CHUNK_SIZE, buffer_size - total_bytes_read);
        rc = fs_read(&file, out_buffer + total_bytes_read, bytes_to_read);
        if (rc < 0) {
            printk("Failed to read from file %s: %d\n", filename, rc);
            fs_close(&file);
            return -1;
        }
        total_bytes_read += bytes_to_read;
    }

    fs_close(&file);
    return total_bytes_read; // Return the number of bytes read
}

static int verify_file_content(const char *fname, const char *expected_content, size_t expected_len) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, fname, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("Failed to open file %s: %d", fname, rc);
        return -1;
    }

    char buffer[READ_CHUNK_SIZE];
    size_t total_bytes_read = 0;

    while ((rc = fs_read(&file, buffer, READ_CHUNK_SIZE)) > 0) {
        for (int i = 0; i < rc; i++) {
            if (buffer[i] != expected_content[total_bytes_read + i]) {
                LOG_ERR("Mismatch at byte %zu: expected 0x%02x, got 0x%02x",
                        total_bytes_read + i, expected_content[total_bytes_read + i], buffer[i]);
                fs_close(&file);
                return -1;
            }
        }
        total_bytes_read += rc;
    }

    if (rc < 0) {
        LOG_ERR("Failed to read the content of the file %s: %d", fname, rc);
        fs_close(&file);
        return -1;
    }

    if (total_bytes_read != expected_len) {
        LOG_ERR("File size mismatch: expected %zu, got %zu", expected_len, total_bytes_read);
        fs_close(&file);
        return -1;
    }

    fs_close(&file);
    return 0;
}



static int append_to_file(const char *filename, const char *data) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_RDWR | FS_O_APPEND);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t length = strlen(data);
    rc = fs_write(&file, data, length);
    if (rc < 0) {
        printk("Failed to write to file %s: %d\n", filename, rc);
        fs_close(&file);
        return -1;
    }

    // rc = fs_sync(&file);
    // if (rc < 0) {
    //     printk("Failed to sync file %s: %d\n", filename, rc);
    //     fs_close(&file);
    //     return -1;
    // }

    fs_close(&file);
    return 0;
}


static int overwrite_file_start(const char *filename, const char *data) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_RDWR);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t data_len = strlen(data);
    rc = fs_write(&file, data, data_len);
    if (rc < 0) {
        printk("Failed to overwrite file %s: %d\n", filename, rc);
        fs_close(&file);
        return -1;
    }

    // rc = fs_sync(&file);
    // if (rc < 0) {
    //     printk("Failed to sync file %s: %d\n", filename, rc);
    //     fs_close(&file);
    //     return -1;
    // }

    fs_close(&file);
    return 0;
}


static int create_and_write_file_in_chunks(const char *filename, size_t total_size) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR | FS_O_APPEND);
    if (rc < 0) {
        printk("Failed to open file %s: %d\n", filename, rc);
        return -1;
    }

    size_t total_bytes_written = 0;
    size_t content_len = strlen(FILE_CONTENT);

    while (total_bytes_written < total_size) {
        size_t bytes_to_write = MIN(WRITE_CHUNK_SIZE, total_size - total_bytes_written);
        size_t bytes_written_this_time = 0;

        while (bytes_written_this_time < bytes_to_write) {
            size_t remaining_bytes_to_write = MIN(content_len, bytes_to_write - bytes_written_this_time);
            rc = fs_write(&file, FILE_CONTENT, remaining_bytes_to_write);
            if (rc < 0) {
                printk("Failed to write to file %s: %d\n", filename, rc);
                fs_close(&file);
                return -1;
            }
            bytes_written_this_time += remaining_bytes_to_write;
        }

        total_bytes_written += bytes_to_write;
    }

    // rc = fs_sync(&file);
    // if (rc < 0) {
    //     printk("Failed to sync file %s: %d\n", filename, rc);
    //     fs_close(&file);
    //     return -1;
    // }

    fs_close(&file);
    return 0;
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
    const struct spi_dt_spec spidev_dt = nand_init();//gets pointer to device from DT
    
    //check if the communication works by reading out the device ID
    res = test_IDs_spi_nand(&spidev_dt);
    if (res != 0) {
        LOG_ERR("Device & Manufacturer ID test failed");
        return -1;
    }

    //nand_flash_init_device(&nand_flash_config, &device_handle); // already initialized
    res = nand_flash_get_sector_size(device_handle, &sector_size);
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
    res = lsdir(nand_mount_fat.mnt_point);
    if (res < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, res);
		return -1;
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

    LOG_INF("Overall, Test 3: Creating file, writing, reading out and deleting");
    char fname[MAX_PATH_LEN];
    struct fs_statvfs sbuf;
    
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

    LOG_INF("Deleting file %s", fname);
    fs_unlink(fname);//deleting file

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
 * @brief Test storing a large file that spans multiple blocks on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_store_large_file(void) {
    LOG_INF("Overall, Test 4: Creating large file, comparing if correctly stored");
    char fname[MAX_PATH_LEN];
    struct fs_statvfs sbuf;
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_LARGE);

    // Create a large content by repeating FILE_CONTENT multiple times
    size_t content_len = strlen(FILE_CONTENT) * CONTENT_REPEAT_COUNT;
    char *large_content = malloc(content_len + 1);
    if (!large_content) {
        LOG_ERR("Failed to allocate memory for large content");
        return -1;
    }

    large_content[0] = '\0';
    for (int i = 0; i < CONTENT_REPEAT_COUNT; i++) {
        strcat(large_content, FILE_CONTENT);
    }

    // Create and write the large file
    int rc = create_and_write_file(fname, large_content);
    if (rc < 0) {
        LOG_ERR("Failed to create and write large file");
        free(large_content);
        return -1;
    }LOG_INF("Large file written to storage");

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


    
    // Verify the content of the large file
    rc = verify_file_content(fname, large_content, content_len); //read and compare in chunks. Otherwise memcmp throws error
    if (rc < 0) {
        LOG_ERR("Content verification failed for large file");
        free(large_content);
        return -1;
    }

    free(large_content);
    return 0;
}


/**
 * @brief Test appending data to a large file and inspect the changes.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_append_data_large_file(void) {
    LOG_INF("Test 5: Appending data to large file and verifying");

    char fname[MAX_PATH_LEN];
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_LARGE);

    // Append new content to the existing large file
    int rc = append_to_file(fname, APPEND_CONTENT);
    if (rc < 0) {
        LOG_ERR("Failed to append to large file");
        return -1;
    }
    LOG_INF("Appended data to large file");

    // Verify the new content
    size_t content_len = strlen(FILE_CONTENT) * CONTENT_REPEAT_COUNT;
    size_t append_content_len = strlen(APPEND_CONTENT);
    size_t new_content_len = content_len + append_content_len;

    char *expected_content = malloc(new_content_len + 1);
    if (!expected_content) {
        LOG_ERR("Failed to allocate memory for expected content");
        return -1;
    }
    
    char *large_content = malloc(content_len + 1);

    large_content[0] = '\0';
    for (int i = 0; i < CONTENT_REPEAT_COUNT; i++) {
        strcat(large_content, FILE_CONTENT);
    }

    strcpy(expected_content, large_content);
    strcat(expected_content, APPEND_CONTENT);

    rc = verify_file_content(fname, expected_content, new_content_len);
    if (rc < 0) {
        LOG_ERR("Content verification failed after appending to large file");
        free(expected_content);
        free(large_content);
        return -1;
    }
    LOG_INF("Content of file with appended data as expected");

    free(expected_content);
    free(large_content);
    return 0;
}

/**
 * @brief Test changing the data of a file and analyze the process on the NAND device.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_change_file_data(void){
    LOG_INF("Test 6: Changing the beginning of the file data and verifying");

    char fname[MAX_PATH_LEN];
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_LARGE);

    // Overwrite the beginning of the file with the sonnet
    int rc = overwrite_file_start(fname, sonnet);
    if (rc < 0) {
        LOG_ERR("Failed to overwrite the beginning of the file");
        return -1;
    }
    LOG_INF("Overwritten the beginning of the file with the sonnet");

    size_t content_len = strlen(FILE_CONTENT) * CONTENT_REPEAT_COUNT;
    size_t append_content_len = strlen(APPEND_CONTENT);
    size_t new_content_len = content_len + append_content_len;

   

    char *expected_content = malloc(new_content_len + 1);
    if (!expected_content) {
        LOG_ERR("Failed to allocate memory for expected content");
        return -1;
    }

    char *large_content = malloc(content_len + 1);

    large_content[0] = '\0';
    for (int i = 0; i < CONTENT_REPEAT_COUNT; i++) {
        strcat(large_content, FILE_CONTENT);
    }
    size_t sonnet_len = strlen(sonnet);
    strcpy(expected_content, sonnet);
    strcat(expected_content, large_content + sonnet_len);
    strcat(expected_content, APPEND_CONTENT);

    // Verify the new content
    rc = verify_file_content(fname, expected_content, new_content_len);
    if (rc < 0) {
        LOG_ERR("Content verification failed after changing the file data");
        free(expected_content);
        free(large_content);
        return -1;
    }

    free(expected_content);
    free(large_content);
    return 0;
}
  

/**
 * @brief Test deleting a file from the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_delete_file(void){
    LOG_INF("Test 7: Deleting a file from the fs and checking");
    struct fs_statvfs sbuf;

    char fname[MAX_PATH_LEN];
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_LARGE);
    fs_unlink(fname);

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

    return 0;
}

/**
 * @brief Test writing to one-eighth of the flash memory.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_write_one_eighth_flash(void) {
    LOG_INF("Test: Writing to one-eighth of the flash memory");
    struct fs_statvfs sbuf;
    struct fs_statvfs sbuf2;

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

    size_t one_eighth_flash_size = sbuf.f_bsize * sbuf.f_blocks / 8;

    char fname2[MAX_PATH_LEN];
    snprintf(fname2, sizeof(fname2), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_ONE_EIGHTH);

    // Write data to file in chunks
    rc = create_and_write_file_in_chunks(fname2, one_eighth_flash_size);
    if (rc < 0) {
        LOG_ERR("Failed to create and write to file");
        return -1;
    }
    LOG_INF("Written one-eighth of the flash memory to file");

    rc = lsdir(nand_mount_fat.mnt_point);
    if (rc < 0) {
        LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
        return -1;
    }

    rc = fs_statvfs(nand_mount_fat.mnt_point, &sbuf2);
    if (rc < 0) {
        LOG_PRINTK("FAIL: statvfs: %d\n", rc);
        return -1;
    }

    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
           " blocks = %lu ; bfree = %lu\n",
           nand_mount_fat.mnt_point,
           sbuf2.f_bsize, sbuf2.f_frsize,
           sbuf2.f_blocks, sbuf2.f_bfree);

    // Delete the file
    rc = fs_unlink(fname2);
    if (rc < 0) {
        LOG_ERR("Failed to delete the file");
        return -1;
    }
    LOG_INF("Deleted the file successfully");

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
/////////////////////////////////////////////       FUNCTIONALITY TESTS END     //////////////////////////

////////////////////////////////////////            PERFORMANCE TESTS START     /////////////////////////////

//measure the speed of writing 1kbit
#define FILE_NAME_SMALL "small_file.txt"
#define FILE_CONTENT2 "Hello, NAND Flash!"
#define CONTENT_REPEAT_COUNT_SMALL (1024 / sizeof(FILE_CONTENT2))

/**
 * @brief Measure the time taken to store 1 KB of data on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */int test_write_read_speed(void) {
    LOG_INF("Test: Creating files of various sizes, measuring time taken for write and read operations");

    // Array of sizes to test
    
    size_t sizes[] = {1, 12, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};
    const char *size_labels[] = {"1 byte", "12 bytes", "128 bytes", "256 bytes", "512 bytes", "1 KB", "2 KB", "4 KB", "8 KB", "16 KB", "32 KB", "64 KB", "128 KB", "256 KB", "512 KB"};
    size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    // Loop through each size and perform the test
    for (size_t i = 0; i < num_sizes; i++) {
        size_t content_len = sizes[i];
        char *content = malloc(content_len + 1);
        if (!content) {
            LOG_ERR("Failed to allocate memory for content of size %zu", content_len);
            return -1;
        }

        // Fill the content with a repeating pattern
        for (size_t j = 0; j < content_len; j++) {
            content[j] = 'A' + (j % 26); // A simple repeating pattern
        }
        content[content_len] = '\0';

        char fname[MAX_PATH_LEN];
        snprintf(fname, sizeof(fname), "%s/%zu_%s", nand_mount_fat.mnt_point, content_len, FILE_NAME_SMALL);
        
        // Measure the time taken to write the file
        int64_t start, end;
        start = k_uptime_get();

        int rc = create_and_write_file_without_sync(fname, content, content_len);
        end = k_uptime_get();
        free(content);
        if (rc < 0) {
            LOG_ERR("Failed to create and write file of size %zu", content_len);
            
            return -1;
        }

        int64_t write_time = end - start;
        LOG_INF("Time taken to write %s file: %lld milliseconds", size_labels[i], write_time);

        // Measure the time taken to read the file
        char *buffer = malloc(content_len + 1);
        if (!buffer) {
            LOG_ERR("Failed to allocate memory for buffer of size %zu", content_len);
          
            return -1;
        }

        start = k_uptime_get();

        int bytes_read = read_file_out_without_logging(fname, buffer, content_len);
        end = k_uptime_get();
        if (bytes_read < 0) {
            LOG_ERR("Failed to read the content of the file %s", fname);
            
            free(buffer);
            fs_unlink(fname);
            return -1;
        }

        int64_t read_time = end - start;
        LOG_INF("Time taken to read %s file: %lld milliseconds", size_labels[i], read_time);

        fs_unlink(fname);

        free(buffer);
    }
    int rc = lsdir(nand_mount_fat.mnt_point);
    if (rc < 0) {
        LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
        return -1;
    }

    return 0;
}

//manual latency test
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int test_write_read_speed_chunks(void) {
    LOG_INF("Test: Creating files of various sizes, measuring time taken for write and read operations");
    int ret;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}


    // Array of sizes to test
    size_t sizes[] = {
        1, 12, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288,
        1048576, 2097152, 4194304, 8388608
    };

    const char *size_labels[] = {
        "1 byte", "12 bytes", "128 bytes", "256 bytes", "512 bytes", "1 KB", "2 KB", "4 KB", "8 KB", 
        "16 KB", "32 KB", "64 KB", "128 KB", "256 KB", "512 KB", "1 MB", "2 MB", "4 MB", "8 MB"
    };
    size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    int64_t write_times[num_sizes];
    int64_t read_times[num_sizes];

    // Loop through each size and perform the test
    for (size_t i = 0; i < num_sizes; i++) {
        size_t content_len = sizes[i];

        char fname[MAX_PATH_LEN];
        snprintf(fname, sizeof(fname), "%s/%zu_%s", nand_mount_fat.mnt_point, content_len, FILE_NAME_SMALL);

        // Measure the time taken to write the file
        int64_t start, end;
        start = k_uptime_get();
        ret = gpio_pin_toggle_dt(&led);
        int rc = create_and_write_file_in_chunks_speed(fname, content_len);
        ret = gpio_pin_toggle_dt(&led);
        end = k_uptime_get();
        if (rc < 0) {
            LOG_ERR("Failed to create and write file of size %zu", content_len);
            return -1;
        }

        write_times[i] = end - start;
        LOG_INF("Time taken to write %s file: %lld milliseconds", size_labels[i], write_times[i]);

        // Measure the time taken to read the file
        start = k_uptime_get();

        rc = read_file_in_chunks(fname, content_len);
        end = k_uptime_get();
        if (rc < 0) {
            LOG_ERR("Failed to read the content of the file %s", fname);
            fs_unlink(fname);
            return -1;
        }

        read_times[i] = end - start;
        LOG_INF("Time taken to read %s file: %lld milliseconds", size_labels[i], read_times[i]);

        fs_unlink(fname);
    }

    int rc = lsdir(nand_mount_fat.mnt_point);
    if (rc < 0) {
        LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
        return -1;
    }
    // Print all results after the tests
    printf("Size,Write Time (ms),Write Data Rate (bytes/s),Read Time (ms),Read Data Rate (bytes/s)\n");
    for (size_t i = 0; i < num_sizes; i++) {
        printf("%s,%lld,%lld\n", size_labels[i], write_times[i], read_times[i]);
    }

    return 0;
}





//Latency tests: Procedure: Measure the time taken for each of these operations individually 
//to understand the delay between initiating an operation and its completion.
//This value can be taken from the previous test, for the speed we need to 

////////////////////////////////////////            PERFORMANCE TESTS END     /////////////////////////////


////////////////////////////////////////            RELIABILITY TESTS START     /////////////////////////////
//wear leveling test, write repeatedly to the flash and log the blocks, fill up the entire flash multiple times
//do this test with logging the output around three hours, use the function test_write_read_speed_chunks and go up to 1024 MB
// ==> basically look at wear leveling in the longterm test


/**
 * marks the block 0 as bad
*/
static int mark_block_as_bad(void)
{
    int ret;
    uint16_t bad_block_indicator = 0;
    uint32_t first_block_page = 0;

    LOG_DBG("mark_bad, block=%u, page=%u, indicator = %04x", 0, first_block_page, bad_block_indicator);

    ret = nand_write_enable();
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = nand_erase_block(first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return -1;
    }

    ret = nand_write_enable();
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = nand_program_load((uint8_t *)&bad_block_indicator, 2048, 2);
    if (ret != 0) {
        LOG_ERR("Failed to program load, error: %d", ret);
        return -1;
    }


    ret = nand_program_execute(first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to execute program on page %u, error: %d", first_block_page, ret);
        return -1;
    }

    while (true) {
        uint8_t status;
        int ret = nand_read_register( REG_STATUS, &status);
        if (ret != 0) {
            LOG_ERR("Error reading NAND status register");
            return -1; 
        }

        if ((status & STAT_BUSY) == 0) {
            break;
        }
        k_sleep(K_MSEC(1)); 
        
    }

    return 0;
}

static int mark_block_as_good(void)
{
    int ret;
    uint16_t bad_block_indicator = 0xFF;
    uint32_t first_block_page = 0;

    LOG_DBG("mark_bad, block=%u, page=%u, indicator = %04x", 0, first_block_page, bad_block_indicator);

    ret = nand_write_enable();
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = nand_erase_block(first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return -1;
    }

    ret = nand_write_enable();
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = nand_program_load((uint8_t *)&bad_block_indicator, 2048, 2);
    if (ret != 0) {
        LOG_ERR("Failed to program load, error: %d", ret);
        return -1;
    }


    ret = nand_program_execute(first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to execute program on page %u, error: %d", first_block_page, ret);
        return -1;
    }

    while (true) {
        uint8_t status;
        int ret = nand_read_register(REG_STATUS, &status);
        if (ret != 0) {
            LOG_ERR("Error reading NAND status register");
            return -1; 
        }

        if ((status & STAT_BUSY) == 0) {
            break;
        }
        k_sleep(K_MSEC(1)); 
        
    }

    return 0;
}

//bad block test, mark a block as bad and check if the system avoids it
int bad_block_test(void){
    int ret;

    //first erase chip ==> starting at block 1, dhara mapping remains
    ret = nand_erase_chip(device_handle);
    if(ret != 0){
        LOG_ERR("Erase chip, error: %d", ret);
        return -1;
    }

    //fill with 0, indicating bad block
    ret = mark_block_as_bad();


    //write data to the flash and check if it recognizes bad block

    //check if dhara recognizes the block (either use first block and erase mapping or use the next one that comes)
    ret = test_store_large_file();
    if(ret == 0){
        LOG_INF("Overall Test 10: Simulation of bad block finished, recognized?");
    }else{
        return -1;
    }

    //look at log if it recognized the bad block

    ret = mark_block_as_good();
    ret = nand_erase_chip(device_handle);
    if(ret != 0){
        LOG_ERR("Erase chip, error: %d", ret);
        return -1;
    }
    
    return ret;
}

//check power loss, is the data still saved?

////////////////////////////////////////            RELIABILITY TESTS END     /////////////////////////////



////////////////////////////////////////            LONG TERM TEST START     /////////////////////////////
//Write and read over days, and look how the data is corrupted.


int faulty_read_write_incidences = 0;

static int delete_file_if_exists(const char *path) {
    struct fs_dirent entry;
    int ret = fs_stat(path, &entry);

    if (ret == 0 && entry.type == FS_DIR_ENTRY_FILE) {
        ret = fs_unlink(path);
        if (ret < 0) {
            LOG_ERR("Failed to delete file %s: %d", path, ret);
            return -1;
        }
        LOG_INF("File %s deleted successfully", path);
    }

    return ret;
}

static void fill_buffer_rand(uint32_t seed, uint8_t *dst, size_t count)
{
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        dst[i] = rand() & 0xFF;  
        //LOG_INF("Index %zu: 0x%02X", i, dst[i]);
    }
}

static void print_buffer(const uint8_t *buf, size_t len) {
    LOG_INF("Buffer:");
    for (size_t i = 0; i < len; i += 40) {
        char line[40 * 3 + 1]; // 40 bytes * 2 hex chars + 1 space + null terminator
        size_t line_len = MIN(40, len - i);
        for (size_t j = 0; j < line_len; j++) {
            snprintf(&line[j * 3], 4, "%02x ", buf[i + j]);
        }
        line[line_len * 3] = '\0'; // Null-terminate the string
        LOG_INF("%s", line);
    }
}


static uint8_t pattern_buf[2048];

static int create_and_write_file_in_chunks_rand(const char *filename, size_t total_size) {
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        LOG_ERR("Failed to open file %s: %d", filename, rc);
        return -1;
    }
    memset(pattern_buf, 0xFF, 2048);
    fill_buffer_rand(PATTERN_SEED, pattern_buf, 2048);

    print_buffer(pattern_buf, sizeof(pattern_buf));


    size_t total_bytes_written = 0;
    size_t content_len = 2048;

    while (total_bytes_written < total_size) {
        size_t bytes_to_write = MIN(WRITE_CHUNK_SIZE, total_size - total_bytes_written);
        size_t bytes_written_this_time = 0;

        while (bytes_written_this_time < bytes_to_write) {
            size_t remaining_bytes_to_write = MIN(content_len, bytes_to_write - bytes_written_this_time);
            rc = fs_write(&file, pattern_buf, remaining_bytes_to_write);
            if (rc < 0) {
                LOG_ERR("Failed to write to file %s: %d", filename, rc);
                fs_close(&file);
                return -1;
            }
            // rc = fs_sync(&file);
            // if (rc < 0) {
            //     LOG_ERR("Failed to sync file %s: %d", filename, rc);
            //     fs_close(&file);
            //     return -1;
            // }
            bytes_written_this_time += remaining_bytes_to_write;
        }

        total_bytes_written += bytes_to_write;
    }

    rc = fs_close(&file);
    if(rc <0){
        LOG_ERR("fault during closing file");
        return -1;
    }
    LOG_INF("File %s written successfully\n", filename);
    return 0;
}


static int write_entire_flash_rand_seed(size_t current_flash_size){
    int rc;    
    //size_t one_eight_flash = current_flash_size ;

    char fname2[MAX_PATH_LEN];
    snprintf(fname2, sizeof(fname2), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_ONE_EIGHTH);
    // delete_file_if_exists(fname2);
    //printk("Deleting existing file %s\n", fname2);
    // Write data to file in chunks
    rc = create_and_write_file_in_chunks_rand(fname2, current_flash_size);
    if (rc < 0) {
        LOG_ERR("Failed to create and write to file %s", fname2);
        return -1;
    }

    return 0;
}

static int check_files(size_t flash_size) {
    int rc;
    size_t one_eight_flash = flash_size;
    LOG_INF("Starting to ckeck files for data corruption");
    //for (int i = 1; i <= 8; i++) {
    char fname[MAX_PATH_LEN];
    snprintf(fname, sizeof(fname), "%s/%s", nand_mount_fat.mnt_point, FILE_NAME_ONE_EIGHTH);

    // Open file
    struct fs_file_t file;
    fs_file_t_init(&file);
    rc = fs_open(&file, fname, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("Failed to open file %s: %d", fname, rc);
        return -1;
    }

    // Read content of the file
    uint8_t buffer[2048];
    size_t bytes_read = 0;
    while (bytes_read < one_eight_flash) {
        size_t chunk_size = MIN(sizeof(buffer), one_eight_flash - bytes_read);
        ssize_t bytes = fs_read(&file, buffer, chunk_size);
        if (bytes < 0) {
            LOG_ERR("Failed to read file %s: %d\n", fname, rc);
            fs_close(&file);
            return -1;
        }
        bytes_read += bytes;

        // Compare with the expected pattern
        srand(PATTERN_SEED);
        for (size_t j = 0; j < bytes; j++) {
            uint8_t expected = rand() & 0xFF; 
            if (buffer[j] != expected) {
                LOG_ERR("Mismatch after %zu/%zu bytes read at index %zu: expected 0x%02X, got 0x%02X",bytes_read, flash_size, j, expected, buffer[j]);
                faulty_read_write_incidences++;
            }
            
        }
        //}
        //LOG_INF("Buffer read:\n%s", buffer);

        // Close the file
        
    }
    fs_close(&file);
    LOG_INF("finished reading files");

    return 0;
}



/**
 * The following tests are performed simultaneously
 * Wear leveling: log which block is written to, retrieve data
 * Log if the expected content is different than the written content, how many times? How many bytes differ?
 * Log how many times there is an ECC
 * Log how many times there is a bad block
*/
int long_term_test(void){
    int rc;
    //first erase chip ==> starting at block 1, dhara mapping remains
    // rc = nand_erase_chip(device_handle);
    // if(rc != 0){
    //     LOG_ERR("Erase chip, error: %d", rc);
    //     return -1;
    // }

    struct fs_statvfs sbuf;
    
    rc = lsdir(nand_mount_fat.mnt_point);
    if (rc < 0) {
		LOG_ERR("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
		return -1;
	}
    rc = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
    if (rc < 0) {
        LOG_ERR("FAIL: statvfs: %d\n", rc);
        return -1;
    }

    size_t current_flash_size = sbuf.f_bsize * sbuf.f_blocks;
    while(true){
        //write entire flash full in 8 files
        rc = write_entire_flash_rand_seed(current_flash_size);
        if (rc < 0) {
            LOG_ERR("Error: unable to write to entire flash");
            return -1;
        }
        
        //read out files and check if they are correct
        rc = check_files(current_flash_size);
        if (rc < 0) {
            LOG_ERR("Error: unable to read entire flash");
            return -1;
        }

        LOG_INF("Overall number of wrong bytes %d", faulty_read_write_incidences);

    }
    //bad blocks, ECC errors and erasing of blocks (wear levelling) have to be logged through the nand.c and oper layer
    //does it overwrite the flash if it is full?



    //if flash filled, check if data is the expected one, if not write difference in log, count how many bytes wrong overall, count how many incidences there are
    //
}

////////////////////////////////////////            LONG TERM TEST END     /////////////////////////////


//Main function in which the other ones are called
int test_all_main_nand_tests(void){
    int ret;
    //is the device on each layer connected/initialized?
    ret = top_device_connected();
    if(ret == 0){
        LOG_INF("Overall Test 1: All modalities sucessful recognized!");
    }else{
        return -1;
    }

   

    // ret = test_create_folder();
    // if(ret == 0){
    //     LOG_INF("Overall Test 2: Creation of folder successful!");
    // }else{
    //     return -1;
    // }

    //  ret = test_create_file();
    // if(ret == 0){
    //     LOG_INF("Overall Test 3: Creation of file, writing and reading successful!");
    // }else{
    //     return -1;
    // }

    // ret = test_store_large_file();
    // if(ret == 0){
    //     LOG_INF("Overall Test 4: Creation of large file, writing and comparing successful!");
    // }else{
    //     return -1;
    // }

    // ret = test_append_data_large_file();
    // if(ret == 0){
    //     LOG_INF("Overall Test 5: Appending to large file data successful");
    // }else{
    //     return -1;
    // }

    // ret = test_change_file_data();
    // if (ret == 0){
    //     LOG_INF("Overall Test 6: Changing data in large file successful");
    // }else{
    //     return -1;
    // }

    // ret = test_delete_file();
    // if (ret == 0){
    //     LOG_INF("Overall Test 7: Deleting the large file is successful");
    // }else{
    //     return -1;
    // }

    // ret = test_write_one_eighth_flash();
    // if (ret == 0){
    //     LOG_INF("Overall Test 8: Writing and deleting to 1/8th of storage");
    // }else{
    //     return -1;
    // }

    LOG_INF("All Functionality tests finished!");


    // ret = test_write_read_speed_chunks();//test_write_read_speed();
    // if(ret== 0){
    //     LOG_INF("Overall Test 9: time for writing and reading 1 byte, 10 byte, 100 byte, 1 kbyte, 10 kbyte, 100 kbyte");
    // }else{
    //     return -1;
    // }

    // ret = bad_block_test(); //==> passed
    // if(ret== 0){
    //     LOG_INF("Overall Test 10: Bad block detections finished");
    // }else{
    //     return -1;
    // }

    ret = long_term_test();
    if(ret== 0){
        LOG_INF("Overall Test 11: Long term test finished???");
    }else{
        LOG_ERR("Overall Test 11: stopping longterm test");
        return -1;
    }

    return 0;
}
