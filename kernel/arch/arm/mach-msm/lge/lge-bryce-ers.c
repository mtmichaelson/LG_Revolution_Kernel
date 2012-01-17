/* arch/arm/mach-msm/board-griffin-ers.c
 *
 * Copyright (C) 2009 LGE, Inc
 * Author: Jun-Yeong Han <j.y.han@lge.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/limits.h>
//<nagireddy.annem@lge.com>24-05-2011 migrating LTE changes from VS910 Froyo start
/* neo.kang@lge.com	10.12.29
 * 0013365 : [kernel] add uevent for LTE ers */
#include <linux/switch.h>
//<nagireddy.annem@lge.com>24-05-2011 migrating LTE changes from VS910 Froyo end
/* neo.kang@lge.com */
#include <linux/slab.h>
/* neo.kang@lge.com	10.12.15. S
 * 0012867 : add the hidden reset
 * */
#include <mach/board_lge.h>
/* neo.kang@lge.com	10.12.15. E */

#define ERS_DRIVER_NAME "ers-kernel"

/* neo.kang@lge.com 11.01.05	
 * 0013574 : bug fix lte crash log and modified code */
#define LTE_WDT_RESET	0x0101
static int copy_frame_buffer(struct notifier_block *this, unsigned long event
		,void *prt);
static atomic_t enable = ATOMIC_INIT(1);
static atomic_t report_num = ATOMIC_INIT(1);

struct ram_console_buffer {
	uint32_t    sig;
	uint32_t    start;
	uint32_t    size;
	uint8_t     data[0];
};

static struct ram_console_buffer *ram_console_buffer = 0;
extern struct ram_console_buffer *get_ram_console_buffer(void);
/* kwangdo.yi@lge.com 10.10.26 S 
  0010357: add ram dump for ERS in AP side 
 */
/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
extern char *ram_console_old_log;
extern size_t ram_console_old_log_size;
#include "../proc_comm.h"
extern char valid_log;
#endif
#endif
/* neo.kang@lge.com	10.12.29. S
 * 0013365 : [kernel] add uevent for LTE ers */
static int ustate = 0;
static ssize_t print_switch_name(struct switch_dev *sdev, char *buf);
static ssize_t print_switch_state(struct switch_dev *sdev, char *buf);
wait_queue_head_t lte_wait_q;
atomic_t lte_log_handled;
static struct switch_dev sdev_lte = {
	.name = "LTE_LOG",
	.print_name = print_switch_name,
	.print_state = print_switch_state,
};
/* neo.kang@lge.com	10.12.29. E */
/* kwangdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com E */

/* kwangdo.yi@lge.com 10.11.25  S 
 * 0011163: [kernel] ERS message display
 */
/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
#include <linux/io.h>

/* neo.kang@lge.com	10.12.15.
 * 0012867 : add the hidden reset */

/* neo.kang@lge.com	10.12.13. S
 * 0012347 : [kernel] add the LTE ers
 * resize misc area to get the LTE log buffer.
 * the size should be lower than 128k because of LTE log buffer
*/
/* neo.kang@lge.com	10.12.13. E */
char *ram_misc_buffer;
 
#endif
#endif

/* neo.kang@lge.com	10.12.15. S
 * 0012867 : add the hidden reset */
#if defined (CONFIG_LGE_HIDDEN_RESET_PATCH)
int hidden_reset_enable = 0;
module_param_named(
		hidden_reset_enable, hidden_reset_enable,
		int, S_IRUGO|S_IWUSR|S_IWGRP);
int on_hidden_reset = 0;

module_param_named(
		on_hidden_reset, on_hidden_reset,
		int, S_IRUGO|S_IWUSR|S_IWGRP);

static int __init check_hidden_reset(char *reset_mode)
{
	if( !strncmp(reset_mode, "on", 2)) {
		on_hidden_reset = 1;
		printk("reboot mode: hidden reset %s\n", "on");
	}

	return 1;
}
__setup("lge.hreset=", check_hidden_reset);

static int copy_frame_buffer(struct notifier_block *this, unsigned long event
		,void *prt)
{
	unsigned char *fb_addr, *f;
	unsigned short *copy_addr;
	void *copy_phys_addr;
	int fb_size = 480*800*2;
	int i, k = 0;

	copy_addr = (unsigned short*)lge_get_fb_copy_virt_addr();
	fb_addr = (unsigned char*)lge_get_fb_addr();
	f = fb_addr;
	printk("%s: copy %x\n", __func__, copy_addr);
	printk("%s: fbad %x\n", __func__, fb_addr);

	for(i=0; i<800*480; i++) {
		copy_addr[i] = (unsigned short)(((f[k]>>3)<<11)|((f[k+1]>>2)<<5)|(f[k+2]>>3));
		k=k+4;
	}
	//memcpy(copy_addr, fb_addr, fb_size);
	lge_set_reboot_reason(copy_phys_addr);

	*((unsigned *)copy_addr) = 0x12345678;
	printk("%s: hidden magic %x\n", __func__, *((unsigned *)copy_addr));

	return NOTIFY_DONE;
}
#endif // CONFIG_LGE_HIDDEN_RESET_PATCH
/* neo.kang@lge.com	10.12.15. E */

/* kwangdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com E */

/* neo.kang@lge.com	10.12.08. S
 * 0012347 : [kernel] add the LTE ers
 * Add the function for getting the LTE crash log.
 * Just modifed the kernel ers like the below.
 * So the comment is removed.
 * Warning : the address is not controlled by kernel.
 *           So you should use carefully the memory.
 * */
/* |                    |
 * | kernel             |
 * |--------------------|
 * | reserved(copy FB)  |
 * |--------------------|
 * | lte log     124k   |
 * |--------------------|
 * | misc area     4k   |
 * |--------------------|
 * | ram console 128k   |
 * |--------------------|
 * | modem              |
 */
#if defined(CONFIG_LGE_LTE_ERS)
/* neo.kang@lge.com	10.12.15.
 * 0012867 : add the hidden reset */

/* current misc_buffer's size is 4k bytes */
struct misc_buffer {
	unsigned char kernel_panic[16];
	unsigned int lte_magic;
	unsigned int lte_reserved;
};

void *ram_lte_log_buf;
static atomic_t lte_enable = ATOMIC_INIT(1);

void lte_panic_report(void)
{
	int fd_dump;
	mm_segment_t oldfs;

#if 0
	value = atomic_read(&lte_enable);

	if( value == 0 )
		return;
#endif

	oldfs = get_fs();
	set_fs(get_ds());

	fd_dump = sys_open("/data/lte_ers_panic", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_dump < 0) {
		printk("%s : can't open the file\n", __func__);
		return;
	}

	printk("### __func__ : %08x\n", (unsigned int)ram_lte_log_buf);
	
	sys_write(fd_dump, ram_lte_log_buf, LTE_LOG_SIZE);
	sys_close(fd_dump);
	set_fs(oldfs);
}
EXPORT_SYMBOL(lte_panic_report);

static ssize_t lte_ers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int value = atomic_read(&lte_enable);
	if (value == 0) {
		printk("The LTE ers of kernel was disabled.\n");
	} else {
		printk("The LTE ers of kernel was enabled.\n");
	}
	return value;
}

static ssize_t lte_ers_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
	atomic_set(&lte_enable, value);

	return size;
}
static DEVICE_ATTR(lte_ers, S_IRUGO | S_IWUSR | S_IWGRP, lte_ers_show, lte_ers_store);

static ssize_t lte_ers_panic_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct misc_buffer *temp;

	temp = (struct misc_buffer *)ram_misc_buffer;

	if( temp->lte_magic == 0x0045544c ) {
		printk("### lte_ers_panic_store \n");
		lte_panic_report();
	} else {
		printk("\n### error : no valid lte log !!\n");
	}
	return size;
}
static DEVICE_ATTR(lte_ers_panic, S_IRUGO | S_IWUSR | S_IWGRP, NULL, lte_ers_panic_store);

int lte_crash_log_in(void *buffer, unsigned int size, unsigned int reserved)
{
	struct misc_buffer *temp;

	temp = (struct misc_buffer *)ram_misc_buffer;
	temp->lte_magic = 0x0045544c;

	if( size > LTE_LOG_SIZE ) {
		printk("%s : buffer size %d is bigger than %d\n",
				__func__, size, LTE_LOG_SIZE);
		return -1;
	}

	memcpy(ram_lte_log_buf, buffer, size);
	printk("%08x, %08x \n", (unsigned int)ram_lte_log_buf, (unsigned int)buffer);
	
	/* save log to the buffer */
	printk("### generate kernel panic for LTE\n");

/* neo.kang@lge.com	10.12.15.
 * 0012867 : add the hidden reset */
#if defined (CONFIG_LGE_HIDDEN_RESET_PATCH)
	atomic_notifier_call_chain(&panic_notifier_list, 0, "[MC] test panic\n");
#endif
	BUG();

	return 0;
}
/* neo.kang@lge.com	10.12.29. S
 * 0013365 : [kernel] add uevent for LTE ers */
int lte_crash_log(void *buffer, unsigned int size, unsigned int reserved)
{
	/* neo.kang@lge.com 11.01.05 S	
	 * 0013574 : bug fix lte crash log and modified code
	 * if hidden reset is set, we should without any condition */
//#if defined (CONFIG_LGE_HIDDEN_RESET_PATCH)
// This feature will be defined in case user variant
// Since lte_log_handled will not be set in the user-signed image ASSERT and WDT_RESET
// will be handled same.
#ifdef CONFIG_LGE_FEATURE_RELEASE
	lte_crash_log_in(buffer, size, reserved);
	return 0;
#endif
	/* if watchDogReset is occured, jsut reset */
	if( reserved == LTE_WDT_RESET) {
		lte_crash_log_in(buffer, size, reserved);
		return 0;
	}
	/* neo.kang@lge.com 11.01.05 E */

	if(ustate == 0) {
		ustate = 1;
		switch_set_state(&sdev_lte, ustate);
	} else {
		ustate = 0;
		switch_set_state(&sdev_lte, ustate);
	}

	wait_event_interruptible(lte_wait_q, atomic_read(&lte_log_handled));

	if (atomic_read(&lte_log_handled))
		lte_crash_log_in(buffer, size, reserved);

	return 0;
}
EXPORT_SYMBOL(lte_crash_log);

static ssize_t lte_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value = 0;
	sscanf(buf, "%d\n", &value);
	printk("[MC] %s: value = %d\n", __func__, value);

	if( value == 1)
		atomic_set(&lte_log_handled, 1);
	else
		atomic_set(&lte_log_handled, 0);

	wake_up(&lte_wait_q);

	return size;
}
static DEVICE_ATTR(lte_cmd, S_IRUGO | S_IWUSR | S_IWGRP, NULL, lte_cmd_store);
/* neo.kang@lge.com	10.12.29. E */
static ssize_t gen_lte_panic_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	void *test_buf = NULL;

	test_buf = kzalloc(4096, GFP_KERNEL);
	printk("### generate kernel panic for LTE (sys file)\n");
	lte_crash_log(test_buf, 4096, 0);

	return size;
}
static DEVICE_ATTR(gen_lte_panic, S_IRUGO | S_IWUSR | S_IWGRP, NULL, gen_lte_panic_store);

/* neo.kang@lge.com 10.12.29. S
 * 0013237 : add the sys file of hidden reset */
#if defined(CONFIG_LGE_HIDDEN_RESET_PATCH)
static ssize_t is_hidden_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", on_hidden_reset);
}

static DEVICE_ATTR(is_hreset, S_IRUGO|S_IWUGO, is_hidden_show, NULL);
#endif
/* neo.kang@lge.com 10.12.29. E */
#endif // CONFIG_LGE_LTE_ERS
/* neo.kang@lge.com	10.12.08. E */

int get_lock_dependencies_report_start(uint32_t start, uint32_t size, uint8_t *data)
{
	int report_start;
	int i;	

	report_start = -1;

	for (i = start - 1; i > -1; --i) {
		if (data[i] == '-') {
			if (!strncmp(&data[i - 25], "-lock_dependencies_report-", 26)) {
				report_start = i + 1;
				break;
			}
		}
	}

	if (i > -1) {
		return report_start;		
	}

	for (i = size - 1; i >= start; --i) {
		if (data[i] == '-') {
			if (!strncmp(&data[i - 25], "-lock_dependencies_report-", 26)) {
				report_start = i + 1;
				break;
			}
		}
	}

	if (i < start) {
		return -1;
	}

	return report_start;
}

void lock_dependencies_report(void)
{
	char filename[NAME_MAX];
	int fd;
	int value;
	mm_segment_t oldfs;

	uint32_t start;
	uint32_t size;
	uint8_t *data;
	int report_start;

	value = atomic_read(&enable);
	if (value == 0) {
		return;
	}

	ram_console_buffer = get_ram_console_buffer();
	if (!ram_console_buffer) {
		return;
	}
	
	start = ram_console_buffer->start; 
	size = ram_console_buffer->size;
	data = ram_console_buffer->data;
	
	report_start = get_lock_dependencies_report_start(start, size, data);
	if(report_start == -1) {
		return;
	}

	sprintf(filename, "/data/ers_lock_dependencies_%d", atomic_read(&report_num));

	oldfs = get_fs();
	set_fs(get_ds());

/* LGE_CHANGES_S [tei.kim@lge.com] 20100109, change file permission */
	fd = sys_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0766);
/* LGE_CHANGES_E [tei.kim@lge.com] 20100109 */
	if (fd < 0) {
		return;
	}
	atomic_inc(&report_num);

	if (report_start < start) {
		sys_write(fd, &data[report_start], start - report_start);
	} else {
		sys_write(fd, &data[report_start], size - report_start);
		sys_write(fd, &data[0], start);
	}
	
	sys_close(fd);
	
	set_fs(oldfs);
}

EXPORT_SYMBOL(lock_dependencies_report);


int get_panic_report_start(uint32_t start, uint32_t size, uint8_t *data)
{
	int report_start;
	int i;	

	report_start = -1;

	for (i = start - 1; i > -1; --i) {
		if (data[i] == 'C') {
			if (!strncmp(&data[i], "CPU:", 4)) {
				report_start = i;
				break;
			}
		}
	}

	if (i > -1) {
		return report_start;		
	}

	for (i = size - 1; i >= start; --i) {
		if (data[i] == 'C') {
			if (!strncmp(&data[i], "CPU:", 4)) {
				report_start = i;
				break;
			}
		}
	}

	if (i < start) {
		return -1;
	}

	return report_start;
}

void panic_report(void)
{
	int fd_dump;
	int value;
	mm_segment_t oldfs;

	uint32_t start;
	uint32_t size;
	uint8_t *data;
	int report_start;
	value = atomic_read(&enable);
	if (value == 0) {
		return;
	}

	ram_console_buffer = get_ram_console_buffer();
	if (!ram_console_buffer) {
		return;
	}
	
	start = ram_console_buffer->start; 
	size = ram_console_old_log_size;
//	data = ram_console_buffer->data;
	data = ram_console_old_log;

	report_start = get_panic_report_start(start, size, data);
	if(report_start == -1) {
		return;
	}

	oldfs = get_fs();
	set_fs(get_ds());
/* kwnagdo.yi@lge.com 10.11.10 S 
 *0010729: change location of kernel panic log file
 * change location from /data/kernel_panic_dump to /data/ers_panic
 */
	fd_dump = sys_open("/data/ers_panic", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_dump < 0) {
		return;
	}

	sys_write(fd_dump, &data[0], size);
	sys_close(fd_dump);
	set_fs(oldfs);
}
EXPORT_SYMBOL(panic_report);

static ssize_t ers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int value = atomic_read(&enable);
	if (value == 0) {
		printk("The ers of kernel was disabled.\n");
	} else {
		printk("The ers of kernel was enabled.\n");
	}
	return value;
}

static ssize_t ers_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
	atomic_set(&enable, value);

	return size;
}

/* LGE_CHANGES_S [tei.kim@lge.com] 20100109, change permission to 666 */
static DEVICE_ATTR(ers, S_IRUGO | S_IWUSR | S_IWGRP, ers_show, ers_store);
/* LGE_CHANGES_E [tei.kim@lge.com] 20100109 */

/* LGE_CHANGES_S [tei.kim@lge.com] 20091207, add kernel panic code */
static ssize_t ers_panic_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
/* kwangdo.yi@lge.com 10.11.25  S 
 * 0011163: [kernel] ERS message display
 */
/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
	char *temp;
	temp = ram_misc_buffer;
#endif
#endif
	/* kwangdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com E */

/* kwangdo.yi@lge.com 10.11.09 S
 * 0010675: auto reboot and save kernel panic message into file automatically
 */
/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
 
	if(valid_log && *temp++ == 0x50 && *temp++ == 0x41 && *temp++ == 0x4E && *temp++ == 0x49 && *temp++==0x43)
	{
/* kwangdo.yi@lge.com 10.11.25  S 
 * 0011163: [kernel] ERS message display
 */
	temp=ram_misc_buffer;
	*temp++ = 0;
	*temp++ = 0;
	*temp++ = 0;
	*temp++ = 0;
	*temp = 0;
/* kwangdo.yi@lge.com E */

/* kwangdo.yi@lge.com 10.11.09 E */
		/* kwangdo.yi@lge.com  10.10.26 S 
0010357: add ram dump for ERS in AP side 
		 */
		printk("### ers_panic_store \n");
		panic_report();
	}
	else printk("\n### error : no valid log !!\n");
#endif
#endif
	/* kwangdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com 10.10.26 E */

	return size;
}

/* LGE_CHANGES_S [tei.kim@lge.com] 20100109, change permission to 666 */
static DEVICE_ATTR(ers_panic, S_IRUGO | S_IWUSR | S_IWGRP, 0, ers_panic_store);
/* LGE_CHANGES_E [tei.kim@lge.com] 20100109 */
/* LGE_CHANGES_E [tei.kim@lge.com] 20091207, add kernel panic code */


/* kwangdo.yi@lge.com  10.10.26 S 
  0010357: add ram dump for ERS in AP side 
 */

/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
static ssize_t gen_panic_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	printk("### generate kernel panic \n");

/* neo.kang@lge.com	10.12.15.
 * 0012867 : add the hidden reset */
#if defined (CONFIG_LGE_HIDDEN_RESET_PATCH)
	atomic_notifier_call_chain(&panic_notifier_list, 0, "[MC] test panic\n");
#endif
	BUG();

	return size;
}
static DEVICE_ATTR(gen_panic, S_IRUGO | S_IWUSR | S_IWGRP, 0, gen_panic_store);

static ssize_t gen_modem_panic_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	printk("### generate modem panic \n");
	msm_proc_comm(PCOM_OEM_MODEM_PANIC_CMD, 0, 0); 

	return size;
}
static DEVICE_ATTR(gen_modem_panic, S_IRUGO | S_IWUSR | S_IWGRP, 0, gen_modem_panic_store);

/*kwangdo.yi@lge.com 10.11.04 imsi for test S */
static ssize_t set_modem_auto_action_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);

	printk("### 0 : download, 1 : reset, 2 : no_action\n");
	printk("### set modem err auto action type : %lu\n", val);
	msm_proc_comm(PCOM_OEM_SET_AUTO_ACTION_TYPE_CMD, &val, 0); 

	return size;
}
static DEVICE_ATTR(set_modem_auto_action, S_IRUGO | S_IWUSR | S_IWGRP, 0, set_modem_auto_action_store);
/*kwangdo.yi@lge.com 10.11.04 imsi for test E */
#endif
#endif
/* kwnagdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com 10.10.26 E */

/* neo.kang@lge.com	10.12.15.
 * 0012867 : add the hidden reset */
#if defined (CONFIG_LGE_HIDDEN_RESET_PATCH)
static struct notifier_block panic_handler_block = {
	.notifier_call	=	copy_frame_buffer,
};
#endif
/* neo.kang@lge.com	10.12.29. S
 * 0013365 : [kernel] add uevent for LTE ers */
#if defined(CONFIG_LGE_LTE_ERS)
static ssize_t print_switch_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "%s\n", "LTE_LOG");
}

static ssize_t print_switch_state(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "%d\n", sdev->state);
}

#if 0
static struct switch_dev sdev_lte = {
	.name = "LTE_LOG",
	.print_name = print_switch_name,
	.print_state = print_switch_state,
};
#endif
#endif
/* neo.kang@lge.com	10.12.29. E */
static int __devinit ers_probe(struct platform_device *pdev)
{
	int ret;
/* kwangdo.yi@lge.com 10.11.25  S 
 * 0011163: [kernel] ERS message display
 */
	size_t buffer_size;
	size_t start;
/* kwangdo.yi@lge.com S */
	ret = device_create_file(&pdev->dev, &dev_attr_ers);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
/* LGE_CHANGES_S [tei.kim@lge.com] 20091207, add kernel panic code */
	ret = device_create_file(&pdev->dev, &dev_attr_ers_panic);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
/* LGE_CHANGES_E [tei.kim@lge.com] 20091207 */

/* kwangdo.yi@lge.com 10.10.26 S 
0010357: add ram dump for ERS in AP side
 */
/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
	ret = device_create_file(&pdev->dev, &dev_attr_gen_panic);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
	
	ret = device_create_file(&pdev->dev, &dev_attr_gen_modem_panic);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
/* neo.kang@lge.com	10.12.12. S
 * 0012347: [kernel] add the LTE ers
 */
#if defined(CONFIG_LGE_LTE_ERS)
	ret = device_create_file(&pdev->dev, &dev_attr_lte_ers);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_lte_ers_panic);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_gen_lte_panic);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
/* neo.kang@lge.com	10.12.29. S
 * 0013365 : [kernel] add uevent for LTE ers */
	ret = device_create_file(&pdev->dev, &dev_attr_lte_cmd);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
	ret = switch_dev_register(&sdev_lte);

	if( ret )
		switch_dev_unregister(&sdev_lte);
/* neo.kang@lge.com	10.12.29. E */
#endif
/* neo.kang@lge.com	10.12.12. E */
#endif
#endif
	/* kwangdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com  10.10.26 E */

/* kwangdo.yi@lge.com  10.11.04 imsi for testS */
	ret = device_create_file(&pdev->dev, &dev_attr_set_modem_auto_action);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
/* kwangdo.yi@lge.com  10.10.26 S */

/* kwangdo.yi@lge.com 10.11.25  S 
 * 0011163: [kernel] ERS message display
 */
/* kwangdo.yi@lge.com 10.11.29 S
 * 0011521: [kernel] ERS enable/disable feature added
 */
#if CONFIG_MACH_LGE_BRYCE
#if defined(CONFIG_ANDROID_RAM_CONSOLE)
	buffer_size = BRYCE_RAM_MISC_SIZE;
	start = BRYCE_RAM_MISC_BASE ;
	printk("\n### ram_misc : got buffer at %zx, size %zx !!\n", start, buffer_size);
	ram_misc_buffer = ioremap(start, buffer_size);
	if(ram_misc_buffer== NULL)
		printk("ram_misc : failed to map memory \n");
	else 
		printk("\n### ram_misc : ok map memory at %08x!!\n", (unsigned int)ram_misc_buffer);

/* neo.kang@lge.com	10.12.12. S
 * 0012347: [kernel] add the lte ers
 */
#if defined(CONFIG_LGE_LTE_ERS)
	printk("\n### ram_lte_log : got buffer at %zx, size %zx !!\n",
			LTE_LOG_START, LTE_LOG_SIZE);
	ram_lte_log_buf = ioremap(LTE_LOG_START, LTE_LOG_SIZE);
	if( ram_lte_log_buf == NULL )
		printk("ram_lte_log_buf : failed to map memory \n");
	else 
		printk("\n### lte_log_buf : ok map memory at %08x!!\n", 
				(unsigned int)ram_lte_log_buf);
/* neo.kang@lge.com	10.12.29. S
 * 0013365 : [kernel] add uevent for LTE ers */
	atomic_set(&lte_log_handled, 0);
	init_waitqueue_head(&lte_wait_q);
/* neo.kang@lge.com	10.12.29. E */
#endif
/* neo.kang@lge.com	10.12.12. E */
#endif
#endif
	/* kwangdo.yi@lge.com 10.11.29 E */
/* kwangdo.yi@lge.com  E */

/* neo.kang@lge.com	10.12.15.
 * 0012867 : Add the hidden reset */
#if defined (CONFIG_LGE_HIDDEN_RESET_PATCH)
/* neo.kang@lge.com 10.12.29. S
 * 0013237 : add the sys file of hidden reset */
	ret = device_create_file(&pdev->dev, &dev_attr_is_hreset);
	if (ret < 0) {
		printk("device_create_file error!\n");
		return ret;
	}
	atomic_notifier_chain_register(&panic_notifier_list, &panic_handler_block);
#endif

	printk("### ers_probe \n");
	return ret;
}

static int __devexit ers_remove(struct platform_device *pdev)
{	
	device_remove_file(&pdev->dev, &dev_attr_ers);
	device_remove_file(&pdev->dev, &dev_attr_ers_panic);

	return 0;
}

static struct platform_driver ers_driver = {
	.probe = ers_probe,
	.remove = __devexit_p(ers_remove),
	.driver = {
		.name = ERS_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init ers_init(void)
{
	return platform_driver_register(&ers_driver);
}
module_init(ers_init);

static void __exit ers_exit(void)
{
	platform_driver_unregister(&ers_driver);
}
module_exit(ers_exit);

MODULE_DESCRIPTION("Griffin Exception Reporting System Driver");
MODULE_AUTHOR("Jun-Yeong Han <j.y.han@lge.com>");
MODULE_LICENSE("GPL");
