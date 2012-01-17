/* arch/arm/mach-msm/board-swift.c
 *
 * Copyright (C) 2009 LGE, Inc.
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

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#define MODULE_NAME	"wm9093"

#define ICODEC_HANDSET_RX	1
#define ICODEC_HEADSET_ST_RX	2
#define ICODEC_HEADSET_MN_RX	3
#define ICODEC_SPEAKER_RX	4
#define	DEBUG_AMP_CTL	1

static uint32_t msm_snd_debug = 1;
module_param_named(debug_mask, msm_snd_debug, uint, 0664);


#if DEBUG_AMP_CTL
#define D(fmt, args...) printk(fmt, ##args)
#else
#define D(fmt, args...) do {} while(0)
#endif

struct amp_data {
	struct i2c_client *client;
};

static struct amp_data *_data = NULL;

int amp_read_register(u8 reg, int* ret)
{
	//ret = swab16(i2c_smbus_read_word_data(_data->client, reg));
	struct i2c_msg	xfer[2];
	u16				data = 0xffff;
	u16				retval;


	xfer[0].addr = _data->client->addr;
	xfer[1].flags = 0;
	xfer[0].len  = 1;
	xfer[0].buf = &reg;

	xfer[1].addr = _data->client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 2;
	xfer[1].buf = (u8*)&data;

	retval = i2c_transfer(_data->client->adapter, xfer, 2);

	*ret =  (data>>8) | ((data & 0xff) << 8);

	return retval;
}

int amp_write_register(u8 reg, int value)
//int amp_write_register(char reg, int value)
{
	//if (i2c_smbus_write_word_data(_data->client, reg, swab16(value)) < 0)
    //			printk(KERN_INFO "========== woonrae: i2c error==================");
	
	int				 err;
	unsigned char    buf[3];
	struct i2c_msg	msg = { _data->client->addr, 0, 3, &buf[0] }; 
	
	buf[0] = reg;
	buf[1] = (value & 0xFF00) >> 8;
	buf[2] = value & 0x00FF;

	if ((err = i2c_transfer(_data->client->adapter, &msg, 1)) < 0){
		return -EIO;
	} 
	else {
		return 0;
	}
}

void set_amp_gain(int icodec_num)
{
        switch(icodec_num) {
                case ICODEC_HANDSET_RX:
                        printk("voc_codec %d does not use the amp\n", icodec_num);
                        break;
                case  ICODEC_HEADSET_ST_RX:
                        printk("voc_codec %d  for HEADSET_ST_RX amp\n", icodec_num);
                                amp_write_register(0x01, 0x000B);
                                amp_write_register(0x02, 0x60C0);
                                amp_write_register(0x16, 0x0001);
                                amp_write_register(0x18, 0x0002);
                                amp_write_register(0x19, 0x0002);
                                amp_write_register(0x18, 0x0102);
                                amp_write_register(0x2D, 0x0040);
                                amp_write_register(0x2E, 0x0010);
                                amp_write_register(0x03, 0x0030);
                                amp_write_register(0x2F, 0x0000);
                                amp_write_register(0x30, 0x0000);
                                amp_write_register(0x16, 0x0000);
                                amp_write_register(0x1C, 0x0039);
                                amp_write_register(0x1D, 0x0139);
                                amp_write_register(0x46, 0x0100);
                                amp_write_register(0x49, 0x0100);
                        break;
                case  ICODEC_SPEAKER_RX:
                        printk("voc_codec %d for SPEAKER_RX amp\n", icodec_num);
                                amp_write_register(0x01, 0x000B);
                                amp_write_register(0x02, 0x6020);
                                amp_write_register(0x1A, 0x0002);
                                amp_write_register(0x1A, 0x0102);
                                amp_write_register(0x36, 0x0004);
                                amp_write_register(0x03, 0x0008);
                                amp_write_register(0x22, 0x0000);
                                amp_write_register(0x03, 0x0108);
                                amp_write_register(0x25, 0x0160);
                                amp_write_register(0x17, 0x0002);
                                amp_write_register(0x01, 0x100B);
                        break;
                default :
                	printk("%s : voc_icodec %d does not support AMP\n", icodec_num);
 
        }
}
EXPORT_SYMBOL(set_amp_gain);


static int bryce_amp_ctl_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct amp_data *data;
	struct i2c_adapter* adapter = client->adapter;
	int err;
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)){
		err = -EOPNOTSUPP;
		return err;
	}

	if (msm_snd_debug & 1)
		printk(KERN_INFO "%s()\n", __FUNCTION__);
	
	data = kzalloc(sizeof (struct amp_data), GFP_KERNEL);
	if (NULL == data) {
			return -ENOMEM;
	}
	_data = data;
	data->client = client;
	i2c_set_clientdata(client, data);
	
	if (msm_snd_debug & 1)
		printk(KERN_INFO "%s chip found\n", client->name);
//while(1){	
	err = amp_write_register(0x00, 0x9093);
	if (err == 0)
		printk(KERN_INFO "AMP INIT OK\n");
	else
		printk(KERN_ERR "AMP INIT ERR\n");
//	msleep(100);
//	}
	return 0;
}

static int bryce_amp_ctl_remove(struct i2c_client *client)
{
	struct amp_data *data = i2c_get_clientdata(client);
	kfree (data);
	
	printk(KERN_INFO "%s()\n", __FUNCTION__);
	i2c_set_clientdata(client, NULL);
	return 0;
}


static struct i2c_device_id bryce_amp_idtable[] = {
	{ "wm9093", 1 },
};

static struct i2c_driver bryce_amp_ctl_driver = {
	.probe = bryce_amp_ctl_probe,
	.remove = bryce_amp_ctl_remove,
	.id_table = bryce_amp_idtable,
	.driver = {
		.name = MODULE_NAME,
	},
};


static int __init bryce_amp_ctl_init(void)
{
	return i2c_add_driver(&bryce_amp_ctl_driver);	
}

static void __exit bryce_amp_ctl_exit(void)
{
	return i2c_del_driver(&bryce_amp_ctl_driver);
}

module_init(bryce_amp_ctl_init);
module_exit(bryce_amp_ctl_exit);

MODULE_DESCRIPTION("SWIFT Amp Control");
MODULE_AUTHOR("Woonrae Cho <woonrae@lge.com>");
MODULE_LICENSE("GPL");
