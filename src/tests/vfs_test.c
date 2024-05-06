
#include <zephyr/kernel.h>
#include <Zephyr/device.h>
#include <Zephyr/drivers/spi.h>
#include <Zephyr/sys/util.h>
#include <zephyr/fs/fs.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/flash.h>

#include <stdio.h>

#include <zephyr/storage/disk_access.h>
#include <ff.h> 
#include "vfs_test.h"


//testing functions to check if fat fs is properly mounted and works
//expecting pattern 55 or aa

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>

static FATFS fat_fs;

/* FAT fs mount info */
static struct fs_mount_t nand_mount_fat = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,//FS_MOUNT_FLAG_NO_FORMAT//FS_MOUNT_FLAG_USE_DISK_ACCESS,
    .storage_dev = (void *) "NAND",  // This should match the name of your disk registered
    .mnt_point = "/NAND:"       // Mount point in the filesystem
};


LOG_MODULE_REGISTER(NAND_mount_test);

/* Matches FF LFN max */
#define MAX_PATH_LEN 255
#define TEST_FILE_SIZE 547

static uint8_t file_test_pattern[TEST_FILE_SIZE];
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

static int nand_increase_infile_value(char *fname)
{
	uint8_t boot_count = 0;
	struct fs_file_t file;
	int rc, ret;

	fs_file_t_init(&file);
	rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
	if (rc < 0) {
		LOG_ERR("FAIL: open %s: %d", fname, rc);
		return rc;
	}

	rc = fs_read(&file, &boot_count, sizeof(boot_count));
	if (rc < 0) {
		LOG_ERR("FAIL: read %s: [rd:%d]", fname, rc);
		goto out;
	}
	LOG_PRINTK("%s read count:%u (bytes: %d)\n", fname, boot_count, rc);

	rc = fs_seek(&file, 0, FS_SEEK_SET);
	if (rc < 0) {
		LOG_ERR("FAIL: seek %s: %d", fname, rc);
		goto out;
	}

	boot_count += 1;
	rc = fs_write(&file, &boot_count, sizeof(boot_count));
	if (rc < 0) {
		LOG_ERR("FAIL: write %s: %d", fname, rc);
		goto out;
	}

	LOG_PRINTK("%s write new boot count %u: [wr:%d]\n", fname,
		   boot_count, rc);

 out:
	ret = fs_close(&file);
	if (ret < 0) {
		LOG_ERR("FAIL: close %s: %d", fname, ret);
		return ret;
	}

	return (rc < 0 ? rc : 0);
}

static void incr_pattern(uint8_t *p, uint16_t size, uint8_t inc)
{
	uint8_t fill = 0x55;

	if (p[0] % 2 == 0) {
		fill = 0xAA;
	}

	for (int i = 0; i < (size - 1); i++) {
		if (i % 8 == 0) {
			p[i] += inc;
		} else {
			p[i] = fill;
		}
	}

	p[size - 1] += inc;
}

static void init_pattern(uint8_t *p, uint16_t size)
{
	uint8_t v = 0x1;

	memset(p, 0x55, size);

	for (int i = 0; i < size; i += 8) {
		p[i] = v++;
	}

	p[size - 1] = 0xAA;
}

static void print_pattern(uint8_t *p, uint16_t size)
{
	int i, j = size / 16, k;

	for (k = 0, i = 0; k < j; i += 16, k++) {
		LOG_PRINTK("%02x %02x %02x %02x %02x %02x %02x %02x ",
			   p[i], p[i+1], p[i+2], p[i+3],
			   p[i+4], p[i+5], p[i+6], p[i+7]);
		LOG_PRINTK("%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[i+8], p[i+9], p[i+10], p[i+11],
			   p[i+12], p[i+13], p[i+14], p[i+15]);

		/* Mark 512B (sector) chunks of the test file */
		if ((k + 1) % 32 == 0) {
			LOG_PRINTK("\n");
		}
	}

	for (; i < size; i++) {
		LOG_PRINTK("%02x ", p[i]);
	}

	LOG_PRINTK("\n");
}

static int nand_binary_file_adj(char *fname)
{
	struct fs_dirent dirent;
	struct fs_file_t file;
	int rc, ret;

	/*
	 * Uncomment below line to force re-creation of the test pattern
	 * file on the FAT FS.
	 */
	/* fs_unlink(fname); */
	fs_file_t_init(&file);

	rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
	if (rc < 0) {
		LOG_ERR("FAIL: open %s: %d", fname, rc);
		return rc;
	}

	rc = fs_stat(fname, &dirent);
	if (rc < 0) {
		LOG_ERR("FAIL: stat %s: %d", fname, rc);
		goto out;
	}

	/* Check if the file exists - if not just write the pattern */
	if (rc == 0 && dirent.type == FS_DIR_ENTRY_FILE && dirent.size == 0) {
		LOG_INF("Test file: %s not found, create one!",
			fname);
		init_pattern(file_test_pattern, sizeof(file_test_pattern));
	} else {
		rc = fs_read(&file, file_test_pattern,
			     sizeof(file_test_pattern));
		if (rc < 0) {
			LOG_ERR("FAIL: read %s: [rd:%d]",
				fname, rc);
			goto out;
		}
		incr_pattern(file_test_pattern, sizeof(file_test_pattern), 0x1);
	}

	LOG_PRINTK("------ FILE: %s ------\n", fname);
	print_pattern(file_test_pattern, sizeof(file_test_pattern));

	rc = fs_seek(&file, 0, FS_SEEK_SET);
	if (rc < 0) {
		LOG_ERR("FAIL: seek %s: %d", fname, rc);
		goto out;
	}

	rc = fs_write(&file, file_test_pattern, sizeof(file_test_pattern));
	if (rc < 0) {
		LOG_ERR("FAIL: write %s: %d", fname, rc);
	}

 out:
	ret = fs_close(&file);
	if (ret < 0) {
		LOG_ERR("FAIL: close %s: %d", fname, ret);
		return ret;
	}

	return (rc < 0 ? rc : 0);
}



// static int nand_flash_erase(unsigned int id)
// {
// 	const struct flash_area *pfa;
// 	int rc;

// 	rc = flash_area_open(id, &pfa);
// 	if (rc < 0) {
// 		LOG_ERR("FAIL: unable to find flash area %u: %d\n",
// 			id, rc);
// 		return rc;
// 	}

// 	LOG_PRINTK("Area %u at 0x%x on %s for %u bytes\n",
// 		   id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,
// 		   (unsigned int)pfa->fa_size);

// 	/* Optional wipe flash contents */
// 	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
// 		rc = flash_area_erase(pfa, 0, pfa->fa_size);
// 		LOG_ERR("Erasing flash area ... %d", rc);
// 	}

// 	flash_area_close(pfa);
// 	return rc;
// }






int test_NAND_flash(void)
{
	char fname1[MAX_PATH_LEN];
	char fname2[MAX_PATH_LEN];
	struct fs_statvfs sbuf;
	int rc;

	LOG_PRINTK("Sample program to r/w files on NAND FAT fs\n");

	rc = fs_mount(&nand_mount_fat);
	if (rc < 0) {
        LOG_ERR("NAND FAT mount failed: %d\n", rc);
		return 0;
	}else{
        LOG_INF("NAND FAT mount successful!");
    }

	snprintf(fname1, sizeof(fname1), "%s/boot_count", nand_mount_fat.mnt_point);
	snprintf(fname2, sizeof(fname2), "%s/pattern.bin", nand_mount_fat.mnt_point);
	

	rc = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (rc < 0) {
		LOG_PRINTK("FAIL: statvfs: %d\n", rc);
		goto out;
	}

	LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

	rc = lsdir(nand_mount_fat.mnt_point);
	if (rc < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", nand_mount_fat.mnt_point, rc);
		goto out;
	}

	rc = nand_increase_infile_value(fname1);
	if (rc) {
		goto out;
	}

	rc = nand_binary_file_adj(fname2);
	if (rc) {
		goto out;
	}

out:
	rc = fs_unmount(&nand_mount_fat);
	LOG_PRINTK("%s unmount: %d\n", nand_mount_fat.mnt_point, rc);
	return 0;
}