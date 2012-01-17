/**/

#include <linux/platform_device.h>
#include <asm/io.h>

extern int lge_write_block(unsigned int bytes_pos, unsigned char *buf, size_t size);
extern int lge_read_block(unsigned int bytes_pos, unsigned char *buf, size_t size);

static int tolk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int result = 0;

	return sprintf(buf, "%d\n", result);
}

static int tolk_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	return count;
}
DEVICE_ATTR(tolk, 0775, tolk_show, tolk_store);

static unsigned int test_result=0;
static int result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char buf1[4];

	lge_read_block(400, buf1, 4);
	test_result = buf1[0];
	printk("%s : %d, %s\n", __func__, test_result, buf1);
	
	return sprintf(buf, "%d\n", test_result);
}

static int result_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)

{
	sscanf(buf, "%d\n", &test_result);
	lge_write_block(400, buf, 4);
	printk("%s : %d, %s\n", __func__, test_result, buf);

	return count;
}
DEVICE_ATTR(result, 0775, result_show, result_store);
static int __devinit lge_tempdevice_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &dev_attr_result);
	if (err < 0)
		printk("%s : cannot create ths sysfs\n", __func__);

	err = device_create_file(&pdev->dev, &dev_attr_tolk);
	if (err < 0)
		printk("%s : cannot create ths sysfs\n", __func__);

	return 0;
}

static struct platform_device lgetemp_device = {
	.name	= "autoall",
	.id		= -1,
};

static struct platform_driver this_driver = {
	.probe	=	lge_tempdevice_probe,
	.driver	=	{
		.name	= "autoall",
	},
};

int __devinit lge_tempdevice_init(void)
{
	printk("%s\n", __func__);
	platform_device_register(&lgetemp_device);

	return platform_driver_register(&this_driver);
}

module_init(lge_tempdevice_init);
MODULE_DESCRIPTION("Just temporal code");
MODULE_AUTHOR("Mooncheol Kang <neo.kang@lge.com>");
MODULE_LICENSE("GPL");
