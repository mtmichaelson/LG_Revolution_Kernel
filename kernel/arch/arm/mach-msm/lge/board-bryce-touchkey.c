/*
 * drivers/input/mise/so240001.c - Touch keypad driver
 *
 * Copyright (C) 2010 LGE, Inc.
 * Author: Cho, EunYoung [ey.cho@lge.com]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <linux/wakelock.h>
#include <mach/msm_i2ckbd.h>
#include <linux/spinlock.h>

#include <mach/board-bryce.h>

#define TSKEY_DEBUG_PRINT	(0)
#define TSKEY_ERROR_PRINT	(1)

#if defined(TSKEY_DEBUG_PRINT)
#define TSKD(fmt, args...) \
			printk(KERN_INFO "D[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define TSKD(fmt, args...)	{};
#endif

#if defined(TSKEY_ERROR_PRINT)
#define TSKE(fmt, args...) \
			printk(KERN_ERR "E[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define TSKE(fmt, args...)	{};
#endif

#if 1 //FIXME temp assign to 5
#define BUT1	KEY_5
#define BUT2	KEY_MENU
#define BUT3	KEY_BACK
#define BUT4	KEY_END
#else
#define BUT1	KEY_SEND
#define BUT2	KEY_MENU
#define BUT3	KEY_BACK
#define BUT4	KEY_END
#endif 

#define MAX_BUT 4

#define TSKEY_REG_INTER			(0x00)
#define TSKEY_REG_GEN			(0x01)
#define TSKEY_REG_BUT_ENABLE		(0x04)
#define TSKEY_REG_SEN1			(0x10)
#define TSKEY_REG_SEN2			(0x11)

#define TSKEY_REG_BUT_STATE1		(0x01)
#define TSKEY_REG_BUT_STATE2		(0x09)

#define TSKEY_REG_DUMMY			(0x00)

#define TSKEY_VAL_SEN1			(0xaa)
#define TSKEY_VAL_SEN2			(0xaa)

#define GPIO_TOUCH_ATTN				180

#define PRESS 	1
#define RELEASE 0

bool ch = RELEASE;
unsigned char old_keycode;

struct so240001_device {
	struct i2c_client		*client;		/* i2c client for adapter */
	struct input_dev		*input_dev;		/* input device for android */
	struct delayed_work		dwork;			/* delayed work for bh */	
	spinlock_t			lock;			/* protect resources */
	int	irq;			/* Terminal out irq number */
};

static struct so240001_device*tskey_pdev = NULL;
static struct workqueue_struct *tskey_wq;

static int
tskey_i2c_write(u8 reg_h, u8 reg_l, u8 val_h, u8 val_l)
{
	int err;
	u8 buf[4];
	struct i2c_msg msg = {
		tskey_pdev->client->addr, 0, sizeof(buf), buf
	};

	buf[0] = reg_h;
	buf[1] = reg_l;
	buf[2] = val_h;
	buf[3] = val_l;

	if ((err = i2c_transfer(tskey_pdev->client->adapter, &msg, 1)) < 0) {
		dev_err(&tskey_pdev->client->dev, "i2c write error\n");
	}

	TSKD("addr_h(0x%x),addr_l(0x%x)\n", reg_h, reg_l);
	TSKD("val_h(0x%x),val_l(0x%x)\n", val_h, val_l);
	return 0;
}

static u16
tskey_i2c_read(u8 reg_h, u8 reg_l, u16 *ret)
{
	int err;
	u8 buf[2];

	struct i2c_msg msg[2] = {
		{ tskey_pdev->client->addr, 0, sizeof(buf), &buf },
		{ tskey_pdev->client->addr, I2C_M_RD, sizeof(buf), ret }
	};

	buf[0] = reg_h;
	buf[1] = reg_l;

	if ((err = i2c_transfer(tskey_pdev->client->adapter, msg, 2)) < 0) {
		dev_err(&tskey_pdev->client->dev, "i2c read error\n");
	}

	//TSKD("addr_h(0x%x),addr_l(0x%x), ret(0x%x)\n", reg_h, reg_l, *ret);
	return 0;

}

static int
tskey_device_initialise(void)
{
	int ret = 0;
	u16 data;
	TSKD("entry\n");

	if (tskey_pdev == NULL) {
		TSKE("tskey device is null\n");
		return -1;
	}

	ret = tskey_i2c_write(TSKEY_REG_DUMMY,  TSKEY_REG_INTER, TSKEY_REG_DUMMY, 0x07);
	if (ret < 0)
		goto end_device_init;

	ret = tskey_i2c_write(TSKEY_REG_DUMMY, TSKEY_REG_GEN, TSKEY_REG_DUMMY, 0x30);
	if (ret < 0)
		goto end_device_init;

	ret = tskey_i2c_write(TSKEY_REG_DUMMY, TSKEY_REG_BUT_ENABLE, TSKEY_REG_DUMMY, 0x0F);
	if (ret < 0)
		goto end_device_init;

	ret = tskey_i2c_write(TSKEY_REG_DUMMY, TSKEY_REG_SEN1, TSKEY_VAL_SEN1, TSKEY_VAL_SEN2);
	if (ret < 0)
		goto end_device_init;

	ret = tskey_i2c_write(TSKEY_REG_DUMMY, TSKEY_REG_SEN2, TSKEY_VAL_SEN1, TSKEY_VAL_SEN2);
	if (ret < 0)
		goto end_device_init;

	ret = tskey_i2c_read(TSKEY_REG_BUT_STATE1, TSKEY_REG_BUT_STATE2, &data);
	TSKD("data(0x109): 0x%x\n", data);

 	return ret;

end_device_init:
		TSKE("failed to initailise\n");
		return ret;

}

static void
tskey_report_event(u16 state)
{
	int keycode;

//	TSKD("entry\n");

	if (state == 0) {
		ch = RELEASE;
		keycode = old_keycode;
	} else if(state == 0x100) {
		ch = PRESS;
		keycode = BUT1;
	} else if(state == 0x200) {
		ch = PRESS;
		keycode = BUT2;
	} else if(state == 0x400) {
		ch = PRESS;
		keycode = BUT3;
	} else if(state == 0x800) {
		ch = PRESS;
		keycode = BUT4;
	} else {
		TSKD("Unknown key type(0x%x)\n", state);
	}

	old_keycode = keycode;

	if(state == 0 || state == 0x100 || state == 0x200 || state == 0x400 ||state == 0x800) {
		input_report_key(tskey_pdev->input_dev, keycode, ch);
		input_sync(tskey_pdev->input_dev);
	}

	//TSKD("exit\n");
}

/*
 * interrupt service routine
 */
static int tskey_irq_handler(int irq, void *dev_id)
{
	struct so240001_device *pdev = dev_id;
	int state;

	spin_lock(&pdev->lock);
	state = gpio_get_value(GPIO_TOUCH_ATTN);
	if(state == 0)
		queue_work(tskey_wq, &pdev->dwork);

/*DO NOT REMOVE THE LOG*/
//	TSKD("ATTN STATE : %d\n", state);
	spin_unlock(&pdev->lock);

	return IRQ_HANDLED;
}

static void
tskey_work_func(struct work_struct *work)
{
	struct so240001_device *pdev = container_of(work, struct so240001_device, dwork.work);
	u16 state;
	int ret;

	ret = tskey_i2c_read(TSKEY_REG_BUT_STATE1, TSKEY_REG_BUT_STATE2, &state);
	//TSKD("press -----> state:0x%x, ret:0x%x\n", state, ret);
	
	tskey_report_event(state);
}

static int tskey_suspend(struct i2c_client *i2c_dev, pm_message_t state);
static int tskey_resume(struct i2c_client *i2c_dev);

/*  ------------------------------------------------------------------------ */
/*  --------------------    SYSFS DEVICE FIEL    --------------------------- */
/*  ------------------------------------------------------------------------ */

static ssize_t
tskey_sen_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u16 data1, data2, data;
	int ret;

	ret = tskey_i2c_read(TSKEY_REG_SEN1, TSKEY_VAL_SEN1, &data1);
	ret = tskey_i2c_read(TSKEY_REG_SEN2, TSKEY_VAL_SEN1, &data2);

	ret = tskey_i2c_read(TSKEY_REG_BUT_STATE1, TSKEY_REG_BUT_STATE2, &data);
	printk("sen1: 0x%x sen2: 0x%x \n", data1, data2);

	return 0;
}

static ssize_t
tskey_sen_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
//	struct i2c_client *client = to_i2c_client(dev);
//	struct so240001_device *pdev = i2c_get_clientdata(client);
	u16 data, data1, data2;
	int ret;

	sscanf(buf, "%d", &data);
	data1 = data<<8;
	data2 = data>>8;
	printk("data: 0x%x (|0x%x|0x%x|)\n", data, data1, data2);

	ret = tskey_i2c_write(0x00, TSKEY_REG_SEN1, data1, data2);
	ret = tskey_i2c_write(0x00, TSKEY_REG_SEN2, data1, data2);

	ret = tskey_i2c_read(0x01, 0x09, &data);
	printk("data(0x109): 0x%x\n", data);
	return count;
}

static ssize_t
tskey_attn_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	int state;

	state = gpio_get_value(GPIO_TOUCH_ATTN);
	printk("GPIO_TOUCH_ATTN state : %d\n", state);

	return 0;
}

static ssize_t
tskey_attn_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	return count;
}

static struct device_attribute tskey_device_attrs[] = {
	__ATTR(sen, S_IRUGO | S_IWUSR, tskey_sen_show, tskey_sen_store),	
	__ATTR(attn, S_IRUGO | S_IWUSR, tskey_attn_show, tskey_attn_store),

};

/*  ------------------------------------------------------------------------ */
/*  --------------------        I2C DRIVER       --------------------------- */
/*  ------------------------------------------------------------------------ */
static int
tskey_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	int i;
	struct SO240001_platform_data	*pdata;
	unsigned keycode = KEY_UNKNOWN;

	TSKD("entry\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)){
		TSKE("it is not support I2C_FUNC_I2C.\n");
		return -ENODEV;
	}

	tskey_pdev = kzalloc(sizeof(struct so240001_device), GFP_KERNEL);
		if (tskey_pdev == NULL) {
			TSKE("failed to allocation\n");
			return -ENOMEM;
		}
	

	INIT_DELAYED_WORK(&tskey_pdev->dwork, tskey_work_func);

	tskey_pdev->client = client;

	i2c_set_clientdata(tskey_pdev->client, tskey_pdev);

	/* allocate input device for transfer proximity event */
	tskey_pdev->input_dev = input_allocate_device();
	if (NULL == tskey_pdev->input_dev) {
			dev_err(&client->dev, "failed to allocation\n");
			goto err_input_allocate_device;
	}

	/* initialise input device for tskey00200F */
	tskey_pdev->input_dev->name = "touch_keypad";
	tskey_pdev->input_dev->phys = "touch_keypad/input3";

	tskey_pdev->input_dev->evbit[0] = BIT_MASK(EV_KEY);
	tskey_pdev->input_dev->keycode = BUT1, BUT2, BUT3, BUT4;
	tskey_pdev->input_dev->keycodesize = sizeof(unsigned short);
	tskey_pdev->input_dev->keycodemax = MAX_BUT;

	keycode = BUT1;
	set_bit(keycode, tskey_pdev->input_dev->keybit);
	keycode = BUT2;
	set_bit(keycode, tskey_pdev->input_dev->keybit);
	keycode = BUT3;
	set_bit(keycode, tskey_pdev->input_dev->keybit);
	keycode = BUT4;
	set_bit(keycode, tskey_pdev->input_dev->keybit);

	/* register input device for tskey */
	ret = input_register_device(tskey_pdev->input_dev);
	if (ret < 0) {
		TSKE("failed to register input\n");
		goto err_input_register_device;
	}

	pdata = tskey_pdev->client->dev.platform_data;
	if (pdata == NULL) {
			TSKE("failed to get platform data\n");
			goto err_tskey_initialise;
	}
	TSKD("input_register_device\n");
	spin_lock_init(&tskey_pdev->lock);

	tskey_pdev->irq = gpio_to_irq(GPIO_TOUCH_ATTN);

	/* register interrupt handler */
	ret = request_irq(tskey_pdev->irq, tskey_irq_handler, IRQF_TRIGGER_FALLING, "so240001_irq", tskey_pdev);
//	ret = request_irq(tskey_pdev->irq, tskey_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "so240001_irq", tskey_pdev);
	if (ret < 0) {
		TSKE("failed to register irq\n");
		goto err_irq_request;
	}

	TSKD("i2c client addr(0x%x)\n", tskey_pdev->client->addr);
	TSKD("ATTN STATE : %d\n", gpio_get_value(GPIO_TOUCH_ATTN));

	for (i = 0; i < ARRAY_SIZE(tskey_device_attrs); i++) {
		ret = device_create_file(&client->dev, &tskey_device_attrs[i]);
		if (ret) {
			goto err_device_create_file;
		}
	}

	/* set up registers according to VOUT output mode */
	ret = tskey_device_initialise();
	if (ret < 0) {
		TSKE("failed to init\n");
		goto err_tskey_initialise;
	}

	TSKD("exit\n");

	return 0;

err_device_create_file:
	while(--i >= 0)
		device_remove_file(&client->dev, &tskey_device_attrs[i]);
err_irq_request:
	input_unregister_device(tskey_pdev->input_dev);
err_tskey_initialise:
err_input_register_device:
	input_free_device(tskey_pdev->input_dev);
err_input_allocate_device:
	kfree(tskey_pdev);

	return ret;
}

static int
tskey_i2c_remove(struct i2c_client *client)
{
	struct so240001_device *pdev = i2c_get_clientdata(client);
	int i;

	TSKD("entry\n");
	//free_irq(pdev->irq, NULL);
	input_unregister_device(pdev->input_dev);
	input_free_device(pdev->input_dev);
	kfree(tskey_pdev);

	for (i = 0; i < ARRAY_SIZE(tskey_device_attrs); i++)
		device_remove_file(&client->dev, &tskey_device_attrs[i]);

	TSKD("exit\n");

	return 0;
}

static int
tskey_i2c_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct so240001_device *pdev = i2c_get_clientdata(i2c_dev);

	TSKD("entry\n");
	
	cancel_delayed_work_sync(&pdev->dwork);
	flush_workqueue(tskey_wq);

	TSKD("exit\n");

	return 0;
}


static int
tskey_i2c_resume(struct i2c_client *i2c_dev)
{
	int ret;

	TSKD("entry\n");

	ret = tskey_device_initialise();
	if (ret < 0) {
		TSKE("failed to init\n");
	}
	TSKD("exit\n");

	return 0;
}

static const struct i2c_device_id tskey_i2c_ids[] = {
		{"so240001", 0 },
		{ },
};

MODULE_DEVICE_TABLE(i2c, tskey_i2c_ids);

static struct i2c_driver tskey_i2c_driver = {
	.probe		= tskey_i2c_probe,
	.remove		= tskey_i2c_remove,
	.suspend	= tskey_i2c_suspend,
	.resume		= tskey_i2c_resume,
	.id_table	= tskey_i2c_ids,
	.driver = {
		.name	= "so240001",
		.owner	= THIS_MODULE,
	},
};

static void __exit tskey_i2c_exit(void)
{
	i2c_del_driver(&tskey_i2c_driver);
	if (tskey_wq)
		destroy_workqueue(tskey_wq);
}

static int __init tskey_i2c_init(void)
{
	int ret;

	TSKD("entry\n");

	tskey_wq = create_singlethread_workqueue("tskey_wq");
	if (!tskey_wq) {
		TSKE("failed to create singlethread workqueue\n");
		return -ENOMEM;
	}

	ret = i2c_add_driver(&tskey_i2c_driver);
	if (ret < 0) {
		TSKE("failed to i2c_add_driver \n");
		destroy_workqueue(tskey_wq);
		return ret;
	}
		TSKD("entry\n");

	return 0;
}

module_init(tskey_i2c_init);
module_exit(tskey_i2c_exit);
	
MODULE_LICENSE("GPL");
