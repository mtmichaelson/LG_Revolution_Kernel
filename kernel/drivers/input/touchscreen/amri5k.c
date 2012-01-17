/*
 * drivers/input/touchscreen/AMRI5K.c
 *
 * Copyright (c) 2009 AVAGO Technologies.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <mach/vreg.h>
#include "amri5k.h"

#define TOUCH_NAME "AMRI5K"

//#define DEBUG_PRINT // Define only for debug mode
#ifdef DEBUG_PRINT
	#define dprintk(f...)	printk("AMRI5K " f)
#else
	#define dprintk(x...)	do { ; } while(0)
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Kernel Mode
//#define MULTITOUCH_KERNEL
//#define TOUCH_POLLING // Define only for polling mode
#define OPTIMIZE_RUNTIME // optimize i2c read operation.
#define AMRI5K_SELF_RECOVERY_CODE // On error recovery code
#define AMRI5K_HW_RESET_CODE // reset touch ic on startup
//#define AMRI5K_I2C_MULTIPLE_READ

// Device Interface
#define AMRI5K_IRQ_GPIO		(107)
#define AMRI5K_RESET_GPIO		(90)
#define AMRI5K_SPI_CS_N	(46)

#define EVENT_SAMPLING_RATE	(HZ/30) // HZ/30-> Real interrupt rate: 18HZ (max: HZ/60)

// Device dependent
#define AMRI5K_X_MAX		(480)
#define AMRI5K_Y_MAX            (800)
#define AMRI5K_FORCE_MAX	(1536)
#define AMRI5K_AREA_MAX		(512)
#define AMRI5K_SLEEP_MODE	(0)
#define AMRI5K_SINGLETOUCH_MODE	(1)
#define AMRI5K_MULTITOUCH_MODE	(2) // 4->2

#define MAX_POINT_NUM		(2) // 4->2
#define TOUCH_SENSITIVITY	(2) // 0:low, 1: mid, 2: high
#ifdef AMRI5K_SELF_RECOVERY_CODE
#define AMRI5K_TOUCH_RECOVERY_CNT (2) // check error count
#endif
#define AMRI5K_SROM_RETRY_COUNT	(3)

static struct input_dev *AMRI5K_ts_input = NULL;

//[*]_________________________________________________________________________________________________[*]
// type define
struct amri5k_point{
	u8 touchid;
	u16 x;
	u16 y;
	u16 force;
	u16 area;
};

struct touch_data_format{
	u8 status;
	u8 nbpoints;
	struct amri5k_point tpd[4]; // touch point data
}; 

typedef enum events{
	KEY_RELEASE_EVENT = 0,
	KEY_DOWN_EVENT,
	KEY_MULTI_RELEASE_EVENT,
	KEY_NONE_EVENT,
}keyevent;

typedef enum touch_sensitivity{
	TOUCH_SENS_LOW,
	TOUCH_SENS_MID,
	TOUCH_SENS_HIGH,
	TOUCH_SENS_MAX
}touch_sens_level;

struct AMRI5K_priv {
	struct spi_device *spi;
	struct input_dev *tdev;
	struct delayed_work poswork;
	int irq;
	int touchmode;
	struct touch_data_format touchdata;
	keyevent key_eType;
};

int CPU_ID = 0; // cpu id to use on dual-core

static int AMRI5K_power_set(unsigned char onoff);
int AMRI5K_SROM_Download(struct spi_device *spi);
static int AMRI5K_InitSromDownload(struct spi_device *spi);

static u8 AMRI5K_spi_read_reg(struct spi_device *spi, u8 reg)
{
	int ret;
	u8 val;
	u8 code = reg;

	ret = spi_write_then_read(spi, &code, 1, &val, 1);

	if (ret < 0) {
		printk(KERN_ERR "spi read error reg:0x%X return:%d", reg, ret);
		return ret;
	}
	
	return val;
}

static u8 AMRI5K_spi_write_reg(struct spi_device *spi, unsigned int reg, unsigned int val)
{
	int ret_val;
	unsigned char buf[2];

	/* MSB must be '1' to indicate write */
	buf[0] = reg | 0x80;
	buf[1] = val;
	ret_val = spi_write_then_read(spi, buf, 2, NULL, 0);

	if(ret_val < 0)
	{
		printk(KERN_ERR "spi write error reg:0x%X return:%d", reg, ret_val);
	}

	return ret_val;
}

static int AMRI5K_TouchSensitivityLevel(struct spi_device *spi, touch_sens_level value)
{
	int ret_value=0;

	dprintk("%s sens_level=%d\n", __func__, value);

	switch(value)
	{
		case TOUCH_SENS_LOW:
			ret_value = AMRI5K_spi_write_reg(spi,0x5C,0x00);   // TOUCH_2_HIGH
			ret_value |= AMRI5K_spi_write_reg(spi,0x5D,0xFA);   // TOUCH_2_LOW   // 250
			ret_value |= AMRI5K_spi_write_reg(spi,0x5E,0x01);   // TOUCH_3_HIGH  // 300
			ret_value |= AMRI5K_spi_write_reg(spi,0x5F,0x2C);   // TOUCH_3_LOW                
			break;

		case TOUCH_SENS_MID:
			ret_value = AMRI5K_spi_write_reg(spi,0x5C,0x00);   // TOUCH_2_HIGH
			ret_value |= AMRI5K_spi_write_reg(spi,0x5D,0xC8);   // TOUCH_2_LOW   // 200
			ret_value |= AMRI5K_spi_write_reg(spi,0x5E,0x00);   // TOUCH_3_HIGH  // 250
			ret_value |= AMRI5K_spi_write_reg(spi,0x5F,0xFA);   // TOUCH_3_LOW                
			break;

		case TOUCH_SENS_HIGH:
			ret_value = AMRI5K_spi_write_reg(spi,0x5C,0x00);   // TOUCH_2_HIGH
			ret_value |= AMRI5K_spi_write_reg(spi,0x5D,0x96);   // TOUCH_2_LOW   // 150
			ret_value |= AMRI5K_spi_write_reg(spi,0x5E,0x00);   // TOUCH_3_HIGH  // 200
			ret_value |= AMRI5K_spi_write_reg(spi,0x5F,0xC8);   // TOUCH_3_LOW                
			break;

		default:
			printk(KERN_ERR "AMRI5K Invaild Touch Sensitivity %d", value);
			ret_value = -1;
			break;
	}

	return ret_value;
}

static int AMRI5K_TouchModeSetting(struct spi_device *spi, u8 touchmode)
{
	dprintk("%s\n", __func__);

	AMRI5K_spi_write_reg(spi, 0x48, touchmode);
	return 0;
}

static int AMRI5K_ReadXYPosition(struct AMRI5K_priv *priv, u16 npoint)
{
	struct spi_device *spi = priv->spi;
	int i;
	u8 read_buf[0x26]={0,};
	u8 ret_value=0;
	static u8 single_touch_keydown = FALSE;
	static u8 multi_touch_release = FALSE;
#ifdef AMRI5K_SELF_RECOVERY_CODE
	static u8 touch_error_count = 0;
#endif

	if(npoint>MAX_POINT_NUM)
	{
		dprintk("Out of range [npoint=%d]\n",npoint);
		return -1;
	}

#ifdef AMRI5K_SELF_RECOVERY_CODE
	ret_value= AMRI5K_spi_read_reg(spi, 0x7B);
	if(ret_value == 0x01)
	{
		AMRI5K_spi_write_reg(spi, 0x7B, 0x00);
	}
#endif

	ret_value = AMRI5K_spi_read_reg(spi, 0x02);
	dprintk("---------------------------------------------------------\n");
	dprintk("GetPosition status(reg 0x2): 0x%x\n", ret_value);

	if((ret_value & 0x12) == 0x12)
	{
#ifdef OPTIMIZE_RUNTIME
#ifdef AMRI5K_SELF_RECOVERY_CODE
		if(touch_error_count != 0)
		{
			touch_error_count = 0;
		}
#endif
		// Data Read mode
		AMRI5K_spi_write_reg(spi, 0x7B, 0x01);
		// Read Touch data
		priv->touchdata.nbpoints = AMRI5K_spi_read_reg(spi, 0x01);
		dprintk("Touch point: %d\n", priv->touchdata.nbpoints);

		if(priv->touchdata.nbpoints >= 1)
		{
#ifdef AMRI5K_I2C_MULTIPLE_READ
      read_buf[0]=0x02 | 0x80;     // bit 7 set: multiple read mode
      if(1 != i2c_master_send(spi, read_buf, 1))
      {
      	printk(KERN_ERR"I2C send fail");
      	return -1;
      }

      if(9 != i2c_master_recv(spi, read_buf, 9))
      {
      	printk(KERN_ERR"I2C read fail");
      	return -1;
      }
#else
			for(i=0; i < 9; i++)
			{
				read_buf[i] = AMRI5K_spi_read_reg(spi, 0x02+i);
			}
#endif
			priv->touchdata.tpd[0].touchid = read_buf[0];
			priv->touchdata.tpd[0].y = ((u16)read_buf[1]<<8) + read_buf[2];   // touch chip x(=width,1024) position
			priv->touchdata.tpd[0].x = ((u16)read_buf[3]<<8) + read_buf[4];   // touch chip y(=height 480) position
			priv->touchdata.tpd[0].force = ((u16)read_buf[5]<<8) + read_buf[6];
			priv->touchdata.tpd[0].area = ((u16)read_buf[7]<<8) + read_buf[8];
			dprintk("Touch ID:0x%X X:%d : Y:%d \n", priv->touchdata.tpd[0].touchid, priv->touchdata.tpd[0].x, priv->touchdata.tpd[0].y);
		}

		if(priv->touchdata.nbpoints >= 2)
		{
#ifdef AMRI5K_I2C_MULTIPLE_READ
      read_buf[0]=0x0B | 0x80;     // bit 7 set: multiple read mode
      if(1 != i2c_master_send(spi, read_buf, 1))
      {
      	printk(KERN_ERR"I2C send fail");
      	return -1;
      }

      if(9 != i2c_master_recv(spi, read_buf, 9))
      {
      	printk(KERN_ERR"I2C read fail");
      	return -1;
      }
#else		
			for(i=0; i < 9; i++)
			{
				read_buf[i] = AMRI5K_spi_read_reg(spi, 0x0B+i);
			}
#endif			
			priv->touchdata.tpd[1].touchid = read_buf[0];
			priv->touchdata.tpd[1].y = ((u16)read_buf[1]<<8) + read_buf[2];   // touch chip x(=width,1024) position
			priv->touchdata.tpd[1].x = ((u16)read_buf[3]<<8) + read_buf[4];   // touch chip y(=height 480) position
			priv->touchdata.tpd[1].force = ((u16)read_buf[5]<<8) + read_buf[6];
			priv->touchdata.tpd[1].area = ((u16)read_buf[7]<<8) + read_buf[8];
			dprintk("Touch ID:0x%X X:%d : Y:%d \n", priv->touchdata.tpd[1].touchid, priv->touchdata.tpd[1].x, priv->touchdata.tpd[1].y);
		}

		// Instruction Mode
		AMRI5K_spi_write_reg(spi,0x7b,0x00);
#ifdef AMRI5K_SELF_RECOVERY_CODE
		ret_value= AMRI5K_spi_read_reg(spi, 0x7b);
		if(ret_value == 0x01)
		{
			AMRI5K_spi_write_reg(spi,0x7b,0x00);
		}
#endif

#else /* OPTIMIZE_RUNTIME */
		// Data Read mode
		AMRI5K_spi_write_reg(spi,0x7b,0x01);

		// Read Touch data
		for(i=0; i < 0x13; i++)   // 0x26:upto 4 fingers, 0x13:upto 2 fingers
		{
			read_buf[i] = AMRI5K_spi_read_reg(spi, i);
		}
		
		// Instruction Mode
		AMRI5K_spi_write_reg(spi,0x7b,0x00);

		priv->touchdata.status = read_buf[0];
		priv->touchdata.nbpoints = read_buf[1];
		dprintk("Touch Status:0x%x Touch points:%d\n",priv->touchdata.status, priv->touchdata.nbpoints);

		priv->touchdata.tpd[0].touchid = read_buf[2];
		priv->touchdata.tpd[0].y = ((u16)read_buf[3]<<8) + read_buf[4];   // touch chip x(=width,1024) position
		priv->touchdata.tpd[0].x = ((u16)read_buf[5]<<8) + read_buf[6];   // touch chip y(=height 480) position
		priv->touchdata.tpd[0].force = ((u16)read_buf[7]<<8) + read_buf[8];
		priv->touchdata.tpd[0].area = ((u16)read_buf[9]<<8) + read_buf[10];

		priv->touchdata.tpd[1].touchid = read_buf[11];
		priv->touchdata.tpd[1].y = ((u16)read_buf[12]<<8) + read_buf[13];   // touch chip x(=width,1024) position
		priv->touchdata.tpd[1].x = ((u16)read_buf[14]<<8) + read_buf[15];   // touch chip y(=height 480) position
		priv->touchdata.tpd[1].force = ((u16)read_buf[16]<<8) + read_buf[17];
		priv->touchdata.tpd[1].area = ((u16)read_buf[18]<<8) + read_buf[19];
// Do not use 3,4 finger data
/*
		priv->touchdata.tpd[2].touchid = read_buf[20];
		priv->touchdata.tpd[2].y = ((u16)read_buf[21]<<8) + read_buf[22];   // touch chip x(=width,1024) position
		priv->touchdata.tpd[2].x = ((u16)read_buf[23]<<8) + read_buf[24];   // touch chip y(=height 480) position
		priv->touchdata.tpd[2].force = ((u16)read_buf[25]<<8) + read_buf[26];
		priv->touchdata.tpd[2].area = ((u16)read_buf[27]<<8) + read_buf[28];

		priv->touchdata.tpd[3].touchid = read_buf[29];
		priv->touchdata.tpd[3].y = ((u16)read_buf[30]<<8) + read_buf[31];   // touch chip x(=width,1024) position
		priv->touchdata.tpd[3].x = ((u16)read_buf[32]<<8) + read_buf[33];   // touch chip y(=height 480) position
		priv->touchdata.tpd[3].force = ((u16)read_buf[34]<<8) + read_buf[35];
		priv->touchdata.tpd[3].area = ((u16)read_buf[36]<<8) + read_buf[37];
*/
#endif /* OPTIMIZE_RUNTIME */

#ifdef DEBUG_PRINT
		for(i=0; i<MAX_POINT_NUM; i++)
		{
			dprintk("%d Touch ID:0x%X X:%d : Y:%d Force:%d Area:%d\n", i, priv->touchdata.tpd[i].touchid, priv->touchdata.tpd[i].x, priv->touchdata.tpd[i].y,priv->touchdata.tpd[i].force, priv->touchdata.tpd[i].area);
		}
#endif
		// Check single touch keydown
		if(priv->touchdata.nbpoints == 1)
		{
			single_touch_keydown = TRUE;
			priv->key_eType = KEY_DOWN_EVENT;
		}
		else if(priv->touchdata.nbpoints >= 2)
		{
			multi_touch_release = TRUE;   // Report multi touch key release event
		}
	}
	else if((ret_value & 0x14) == 0x14)
	{
/*eycho : TEST*/
//printk("0x02 Registar----->ret_value: 0x%x / &0x14: 0x%x\n", ret_value, ret_value & 0x14);
		if(single_touch_keydown == TRUE) 
		{
			single_touch_keydown = FALSE;
			priv->key_eType = KEY_RELEASE_EVENT;
		}

		if(multi_touch_release == TRUE) 
		{
			multi_touch_release = FALSE;
			priv->key_eType = KEY_MULTI_RELEASE_EVENT;
		}
	}
#ifdef AMRI5K_SELF_RECOVERY_CODE
	else if((ret_value & 0x80) == 0x80)   // Interrupt always occur. Bit 7 is watch dog reset
	{
		if(touch_error_count > AMRI5K_TOUCH_RECOVERY_CNT)
		{
			touch_error_count = 0;
			AMRI5K_spi_write_reg(spi,0x7a,0xaa);   // Standby mode
			AMRI5K_spi_write_reg(spi,0x7a,0xdd);   // Resume
			msleep(10);
			printk(KERN_ERR "Touch IC Error. Aways Int: 0x%d\n", ret_value);            
		}
	}
	else if((ret_value & 0x02) == 0x02)   // Interrupt always HIGH.
	{
		if(touch_error_count > AMRI5K_TOUCH_RECOVERY_CNT)
		{
			touch_error_count = 0;    
			AMRI5K_spi_write_reg(spi,0x7a,0xaa);   // Standby mode
			AMRI5K_spi_write_reg(spi,0x7a,0xdd);   // Resume
			msleep(10);
			printk(KERN_ERR "Touch IC Error. Maintain Int High: 0x%d\n", ret_value);                        
		}
	}
#endif
	return 0;
}
 
static void AMRI5K_read_touch_event_work(struct work_struct *work)
{
	struct AMRI5K_priv *priv = container_of(work, struct AMRI5K_priv, poswork.work);
	struct touch_data_format *tdata = &priv->touchdata;
	u8 touchid = 0;
	int i=0;
    
#ifdef MULTITOUCH_KERNEL
	AMRI5K_ReadXYPosition(priv, priv->touchmode);
#else
	AMRI5K_ReadXYPosition(priv, 1);
#endif
	dprintk("nbpoints %d\n", tdata->nbpoints);
#ifdef MULTITOUCH_KERNEL
	if(tdata->nbpoints > 0)    
	{
		while(i < tdata->nbpoints)
		{
			touchid = tdata->tpd[i].touchid & 0x7F;   // Bit[7] - Touch point state, 1-real touch point, 0-hovering touch point
			// boundray check
			if (touchid >=1 && touchid <=5 &&
				tdata->tpd[i].x >= 0 && tdata->tpd[i].x <= AMRI5K_X_MAX &&
				tdata->tpd[i].y >= 0 && tdata->tpd[i].y <= AMRI5K_Y_MAX &&
				tdata->tpd[i].force >= 0 && tdata->tpd[i].force <= AMRI5K_FORCE_MAX &&
				tdata->tpd[i].area >= 0 && tdata->tpd[i].area <= AMRI5K_AREA_MAX)
			{
				input_report_abs(priv->tdev, ABS_MT_TRACKING_ID, touchid);
				input_report_abs(priv->tdev, ABS_MT_POSITION_X, tdata->tpd[i].x);
            input_report_abs(priv->tdev, ABS_MT_POSITION_Y, tdata->tpd[i].y);
				input_report_abs(priv->tdev, ABS_MT_TOUCH_MAJOR, tdata->tpd[i].force);
				input_report_abs(priv->tdev, ABS_MT_WIDTH_MAJOR, tdata->tpd[i].area);
				input_mt_sync(priv->tdev);
				dprintk("MULTI Touch: %d\n", i);
			}
			i++;
		}

		input_sync(priv->tdev);

		if(tdata->nbpoints > 1)
		{
			tdata->nbpoints = 0;   // init fot next step
			dprintk("MULTI Touch return\n");   // test
			return;
		}
	}
#endif

	if(priv->key_eType == KEY_DOWN_EVENT)
	{
		priv->key_eType = KEY_NONE_EVENT;   // Initial event

		// boundray check
		if (tdata->tpd[0].x >= 0 && tdata->tpd[0].x <= AMRI5K_X_MAX &&
			tdata->tpd[0].y >= 0 && tdata->tpd[0].y <= AMRI5K_Y_MAX)
		{
        
        input_report_abs(priv->tdev, ABS_X, tdata->tpd[0].x);
        input_report_abs(priv->tdev, ABS_Y, tdata->tpd[0].y);
		input_report_key(priv->tdev, BTN_TOUCH, 1);
			input_sync(priv->tdev);
/*eycho: TEST*/
//printk("touch(x)---> tdata->tpd[0].x: 0x%x \n", tdata->tpd[0].x);
//printk("touch(y)---> tdata->tpd[0].y: 0x%x \n", tdata->tpd[0].y);
//printk("Single Key Down\n");
		}
	}
	else if(priv->key_eType == KEY_RELEASE_EVENT)
	{
		priv->key_eType = KEY_NONE_EVENT;   // Initial event

		input_report_key(priv->tdev, BTN_TOUCH, 0);
		input_sync(priv->tdev);

//printk("Single Key Release\n");
	}
#ifdef MULTITOUCH_KERNEL
	else if(priv->key_eType == KEY_MULTI_RELEASE_EVENT)
	{
		priv->key_eType = KEY_NONE_EVENT;   // Initial event

		input_report_key(priv->tdev, BTN_TOUCH, 1);
		input_sync(priv->tdev);

		input_report_key(priv->tdev, BTN_TOUCH, 0);
		input_sync(priv->tdev);
		dprintk("Multi Key Release\n");
	}
#endif	
	else
	{
		dprintk("Single Ignore Event\n");
	}

	tdata->nbpoints = 0;   // init fot next step

#if 0 //TEST
	schedule_delayed_work_on(CPU_ID, &priv->poswork, EVENT_SAMPLING_RATE);
	dprintk("Single Ignore Event %d\n", EVENT_SAMPLING_RATE);
#endif 
	
}

void AMRI5K_HwReset(void)
{
	int ret =0;

	ret = gpio_tlmm_config(GPIO_CFG(AMRI5K_SPI_CS_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	if (ret) {
			printk(KERN_ERR "%s: gpio_tlmm_config=%d\n",__func__, ret);
	}
	gpio_set_value(AMRI5K_SPI_CS_N, 1);	

	ret = gpio_tlmm_config(GPIO_CFG(AMRI5K_RESET_GPIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	if (ret) {
			printk(KERN_ERR "%s: gpio_tlmm_config=%d\n",__func__, ret);
	}

	gpio_set_value(AMRI5K_RESET_GPIO, 1);	
	udelay(100);

#if defined(LG_HW_REV1) || defined(LG_HW_REV2)
	AMRI5K_power_set(TRUE);
#endif

	gpio_set_value(AMRI5K_RESET_GPIO, 0);
	udelay(100);
	gpio_set_value(AMRI5K_RESET_GPIO, 1);
	udelay(100);	
}

int AMRI5K_SROM_Download(struct spi_device *spi)
{
	int ret_value=0;
	int cnt=0;

	dprintk("SROM Download Start\n");
	// Disable Watchdog
	AMRI5K_spi_write_reg(spi, 0x7D, 0xAD);
	// Clear BOOT_STATE
	AMRI5K_spi_write_reg(spi, 0x0E, 0x00);
	// Enable download 
	AMRI5K_spi_write_reg(spi, 0x08, 0x2B);
	// Download patchcode
	cnt = 0;
	
	//eycho
	do{
		AMRI5K_spi_write_reg(spi, 0x09, amri5k_srom_code[cnt]);
		udelay(15); // Device need to wait time during patch download.
		cnt++;
	} while(cnt < sizeof(amri5k_srom_code));
	
	dprintk("SROM code length: %d\n", cnt); 
	msleep(30);   // Wait until boot up

	// Check download successful?
	cnt = 0;
	
	//eycho
	do{
		ret_value = AMRI5K_spi_read_reg(spi, 0x0E);
		dprintk("SROM Download status:0x%x\n", ret_value);
		
		if((ret_value & 0x1F) == 0x03)
		{
			break;
		}
		else if((ret_value & 0x04) == 0x04)   // Bit 2 High: Download failed
		{
			printk(KERN_ERR "SROM Download status error:0x%x\n", ret_value);
			return -1;
		}
		if(++cnt > 100) // Check time out
		{
			printk(KERN_ERR "SROM Download time out:0x%x\n", ret_value);
			return -1;
		}
		msleep(10);
	}while(1);

	// Enable watchdog
	AMRI5K_spi_write_reg(spi, 0x7D, 0x00);
	dprintk("SROM Download Complete!!\n");

	// Read SROM Version == F/W Version
	ret_value = AMRI5K_spi_read_reg(spi, 0x01);
	dprintk("F/W(SROM) version:0x%x\n", ret_value);

	if(ret_value != AMRI5K_SROM_VERSION)
	{
		printk(KERN_ERR "Mismatch F/W(SROM) version:0x%X", ret_value);
		return -1;
	}

	return 0; // Download success
}

static int AMRI5K_InitSromDownload(struct spi_device *spi)
{
	int ret_value=0;
	int bootCnt=0, retryCnt=0;

	do
	{
		AMRI5K_HwReset();  

		// Software Reset
		AMRI5K_spi_write_reg(spi, 0x7A, 0xAA);
		AMRI5K_spi_write_reg(spi, 0x7A, 0xBB);

		// Wait for boot complete
		bootCnt = 0;

		//eycho
		do{
			msleep(20);     						
			ret_value = AMRI5K_spi_read_reg(spi, 0x0E);
			//dprintk("Wait for boot complete:0x%x bootCnt:%d\n", ret_value, bootCnt);
		} while(!(ret_value&0x1) && (++bootCnt<20));

		// Read Product ID 
		ret_value = AMRI5K_spi_read_reg(spi, 0x00);
		dprintk("Product ID:0x%X\n",ret_value);
		ret_value = AMRI5K_spi_read_reg(spi, 0x01);
		dprintk("HW REV ID:0x%X\n",ret_value);

		ret_value = AMRI5K_SROM_Download(spi);
		if(ret_value < 0)
		{
			printk(KERN_ERR "AMRI5K SROM Download fail. Retry Cnt:%d\n",retryCnt);
		}
	}while((ret_value < 0) && (++retryCnt < AMRI5K_SROM_RETRY_COUNT));   // Retry download

	if(retryCnt >= AMRI5K_SROM_RETRY_COUNT)
	{
		printk(KERN_ERR "AMRI5K SROM fail.\n");
		return -1;
	}

#if 1
	dprintk("Register Initialize Start\n");
//[Initialization Start ] -------------------------------------------------------------------------------
	// Auto calibration enable ?
	AMRI5K_spi_write_reg(spi,0x0F,0x7A);
	// Set the motion pin plarity in INT_DEASSERT 0x0d
	AMRI5K_spi_write_reg(spi,0x0D,0xDF);   // Change Negative Edge 0xCF -> 0xDF
	// Set the interrupt source in INT_MASK 0x07
	AMRI5K_spi_write_reg(spi,0x07,0x06);   // 0x02->0x06 Touch status change Bit2
	// Set STATUS bit clear setting in AUTO_CLEAR 0x06
	AMRI5K_spi_write_reg(spi,0x06,0x3F);
	// Motion Report CTL
	AMRI5K_spi_write_reg(spi,0x1E,0x04);   // Hover touch point report off 0x00 -> 0x04
	// DMOD_CLOCK
	AMRI5K_spi_write_reg(spi,0x40,0x39);   // 0x38-> 0x39
	// SENSE_MAP 0x0A
	AMRI5K_spi_write_reg(spi,0x0A,0x00);
//[Initialization End ] --------------------------------------------------------------------------------
    
//[Resolution Start ] ----------------------------------------------------------------------------------
	// Load screen display value of row(0x41), column(0x42), height(0x43+44), witdth(0x45+46)
	AMRI5K_spi_write_reg(spi,0x43,0x01);   // #define AMRI5K_X_MAX (480) => 0x01E0
	AMRI5K_spi_write_reg(spi,0x44,0xE0);

	AMRI5K_spi_write_reg(spi,0x45,0x03);   // #define AMRI5K_Y_MAX (1024) => 0x0400
	AMRI5K_spi_write_reg(spi,0x46,0x20);
//[Resolution End] -------------------------------------------------------------------------------------

//[Adjust frame rate Start] ----------------------------------------------------------------------------
//	AMRI5K_spi_write_reg(spi,0x22,0x05);   // Line Scan rate
//	AMRI5K_spi_write_reg(spi,0x26,0x12);   // Rest1 frame rate
//	AMRI5K_spi_write_reg(spi,0x2A,0x22);   // Rest2 frame rate
//	AMRI5K_spi_write_reg(spi,0x2E,0x44);   // Rest3 frame rate
//[Adjust frame rate End] ------------------------------------------------------------------------------

//[Thumb/Cheek Detection Start ] -----------------------------------------------------------------------
	AMRI5K_spi_write_reg(spi,0x1A,0x05);   // Cheek Cells - 0x06->0x05
	AMRI5K_spi_write_reg(spi,0x1B,0x50);   // Cheek PCT
	AMRI5K_spi_write_reg(spi,0x1C,0x20);   // Cheek Total Cells
//[Thumb/Cheek Detection End ] -------------------------------------------------------------------------

//[Touch Threshold Start ] -----------------------------------------------------------------------------
	AMRI5K_spi_write_reg(spi,0x67,0x02);   // Max Touch
	AMRI5K_spi_write_reg(spi,0x68,0xEE);   // Max Touch
	// Tunning

	AMRI5K_spi_write_reg(spi,0x0a,0x04);   // sensor mapping
#if 1	
	AMRI5K_spi_write_reg(spi,0x5A,0);   // TOUCH_1_HIGH
	AMRI5K_spi_write_reg(spi,0x5B,120);   // TOUCH_1_LOW
	AMRI5K_spi_write_reg(spi,0x5C,0);   // TOUCH_2_HIGH
	AMRI5K_spi_write_reg(spi,0x5D,150);   // TOUCH_2_LOW
	AMRI5K_spi_write_reg(spi,0x5E,0);   // TOUCH_3_HIGH
	AMRI5K_spi_write_reg(spi,0x5F,200);   // TOUCH_3_LOW
#else
	AMRI5K_spi_write_reg(spi,0x5A,0);   // TOUCH_1_HIGH
	AMRI5K_spi_write_reg(spi,0x5B,100);   // TOUCH_1_LOW
	AMRI5K_spi_write_reg(spi,0x5C,0);   // TOUCH_2_HIGH
	AMRI5K_spi_write_reg(spi,0x5D,120);   // TOUCH_2_LOW
	AMRI5K_spi_write_reg(spi,0x5E,0);   // TOUCH_3_HIGH
	AMRI5K_spi_write_reg(spi,0x5F,150);   // TOUCH_3_LOW

#endif
//[Touch Threshold End ] -------------------------------------------------------------------------------

// Noise tuning 
	AMRI5K_spi_write_reg(spi,0x75,0x08);   // NAV Filter // 0x08 -> 0x09 NV0 set. NV0: Increase panel sampling. NV1:Use higher than normal output data averaging
// Touch point
	AMRI5K_spi_write_reg(spi,0x48,0x02);   // TPOINT // 0x04 -> 0x02. Report 2 fingers only.
// Orientation
	AMRI5K_spi_write_reg(spi,0x0C,0x00);   // Flipping XY 
// Edge stretch factor
	AMRI5K_spi_write_reg(spi,0x13,0x18);   // Report Y-18 at the bottom edge of the panel

	ret_value = AMRI5K_spi_read_reg(spi,0x02);
#endif 	
	dprintk("Register Initialize End Status:0x%X\n",ret_value);
	return 0;
}

static irqreturn_t AMRI5K_isr(int irq, void *dev_id)
{
	struct AMRI5K_priv *priv = dev_id;
	schedule_delayed_work_on(CPU_ID, &priv->poswork, EVENT_SAMPLING_RATE);
	CPU_ID = !CPU_ID; // toggle cpu to use next time

	return IRQ_HANDLED;
}

static int AMRI5K_open(struct input_dev *dev)
{
	struct AMRI5K_priv *priv = input_get_drvdata(dev);

	/* enable controller */
	AMRI5K_TouchSensitivityLevel(priv->spi, TOUCH_SENSITIVITY);
	AMRI5K_TouchModeSetting(priv->spi, priv->touchmode);
	dprintk("open\n");

	return 0;
}

static void AMRI5K_close(struct input_dev *dev)
{
	struct AMRI5K_priv *priv = input_get_drvdata(dev);

	/* cancel pending work and wait for AMRI5K_read_touch_event_work() to finish */
	cancel_delayed_work_sync(&priv->poswork);

	AMRI5K_TouchModeSetting(priv->spi,AMRI5K_SLEEP_MODE);
	dprintk("close\n");
}

static int AMRI5K_power_set(unsigned char onoff)
{
	int ret = 0;
	struct vreg *touch_io_vreg;
	struct vreg *touch_vdd_vreg;
	
#if defined(LG_HW_REV1)
	touch_io_vreg = vreg_get(0, "gp9"); //HW VREG_L12
	touch_vdd_vreg = vreg_get(0, "gp6"); //HW VREG_L15
#endif 
#if defined(LG_HW_REV2)
	touch_io_vreg = vreg_get(0, "wlan"); //HW VREG_L13
	touch_vdd_vreg = vreg_get(0, "gp9"); //HW VREG_L12
#endif 

#if defined(LG_HW_REV1) || defined(LG_HW_REV2)
	if (onoff) {
		ret = vreg_set_level(touch_io_vreg, 3000); 
		ret = vreg_enable(touch_io_vreg);
		if (ret != 0) {
			printk("touch_io_vreg failed.\n");
			return -1;
		}		
		mdelay(3);
		ret = vreg_set_level(touch_vdd_vreg, 1800);
		ret = vreg_enable(touch_vdd_vreg);
		if (ret != 0) {
			printk("touch_vdd_vreg failed.\n");
			return -1;
		}		
		mdelay(30);
		
	} else {
		ret = vreg_set_level(touch_io_vreg, 0);
		ret = vreg_disable(touch_io_vreg);
		ret = vreg_set_level(touch_vdd_vreg, 0);
		ret = vreg_disable(touch_vdd_vreg);
	}
#endif

	return ret;
}

static int AMRI5K_probe(struct spi_device *spi)
{
	struct AMRI5K_priv *priv;
	int error;
	int irq;

	printk(KERN_INFO"probing AMRI5K capacitive touch driver\n");
	
	dprintk("Build Date:%s Time:%s\n",__DATE__,__TIME__);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		printk(KERN_ERR"failed to allocate driver data\n");
		error = -ENOMEM;
		goto err0;
	}

	dev_set_drvdata(&spi->dev, priv);

	AMRI5K_ts_input = input_allocate_device();
	if (!AMRI5K_ts_input) {
		printk(KERN_ERR"Failed to allocate input device.\n");
		error = -ENOMEM;
		goto err1;
	}	

#ifdef MULTITOUCH_KERNEL
	if(AMRI5K_touchmode == AMRI5K_MULTITOUCH_MODE)
	{
		input_set_abs_params(AMRI5K_ts_input, ABS_MT_POSITION_X, 0, AMRI5K_X_MAX, 0, 0);
		input_set_abs_params(AMRI5K_ts_input, ABS_MT_POSITION_Y, 0, AMRI5K_Y_MAX, 0, 0);
		input_set_abs_params(AMRI5K_ts_input, ABS_MT_TRACKING_ID, 0, 5, 0, 0);
		input_set_abs_params(AMRI5K_ts_input, ABS_MT_TOUCH_MAJOR, 0, AMRI5K_FORCE_MAX, 0, 0);
		input_set_abs_params(AMRI5K_ts_input, ABS_MT_WIDTH_MAJOR, 0, AMRI5K_AREA_MAX, 0, 0);

		input_set_abs_params(AMRI5K_ts_input, ABS_X, 0, AMRI5K_X_MAX, 0, 0);
		input_set_abs_params(AMRI5K_ts_input, ABS_Y, 0, AMRI5K_Y_MAX, 0, 0);
		dprintk("MULTI TOUCH Mode Setting");
	}
	else
#endif	
	{
		input_set_abs_params(AMRI5K_ts_input, ABS_X, 0, AMRI5K_X_MAX, 0, 0);
		input_set_abs_params(AMRI5K_ts_input, ABS_Y, 0, AMRI5K_Y_MAX, 0, 0);
		dprintk("SINGLE TOUCH Mode Setting");
	}

	error = AMRI5K_InitSromDownload(spi);
	if(error)
	{
		printk(KERN_ERR"failed to initial AMRI5K\n");
		goto err1;
	}

	AMRI5K_ts_input->name = "AMRI5K";
	AMRI5K_ts_input->phys = "AMRI5K/input0";
	AMRI5K_ts_input->id.bustype = BUS_HOST;	
	AMRI5K_ts_input->dev.parent = &spi->dev;
	AMRI5K_ts_input->open = AMRI5K_open;
	AMRI5K_ts_input->close = AMRI5K_close;

	AMRI5K_ts_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	AMRI5K_ts_input->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
	AMRI5K_ts_input->absbit[BIT_WORD(ABS_MISC)] = BIT_MASK(ABS_MISC);
	AMRI5K_ts_input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	input_set_drvdata(AMRI5K_ts_input, priv);

	priv->touchmode = *(int*)spi->dev.platform_data;
	priv->spi = spi;
	priv->tdev = AMRI5K_ts_input;
	INIT_DELAYED_WORK(&priv->poswork, AMRI5K_read_touch_event_work);

	error = input_register_device(AMRI5K_ts_input);
	if (error) {
		printk(KERN_ERR"amri5k: failed to register input\n");
		goto err1;
	}

#ifndef TOUCH_POLLING
	error = gpio_request(AMRI5K_IRQ_GPIO, "amri5k_irq");
	if (error) {
		printk(KERN_ERR"Unable to request touchscreen GPIO.\n");
		goto err2;
	}

	gpio_direction_input(AMRI5K_IRQ_GPIO);
	irq = gpio_to_irq(AMRI5K_IRQ_GPIO);
	if (irq < 0) {
		error = irq;
		printk(KERN_ERR"Unable to request gpio irq. err=%d\n", error);
		gpio_free(AMRI5K_IRQ_GPIO);
		goto err2;
	}

	priv->irq = irq;

	error = request_irq(priv->irq, AMRI5K_isr, IRQF_TRIGGER_FALLING, TOUCH_NAME, priv);

	if (error) {
		printk (KERN_ERR"unable to claim irq %d: err %d\n", irq, error);
		gpio_free(AMRI5K_IRQ_GPIO);
		goto err2;
	}
#else
	schedule_delayed_work_on(CPU_ID, &priv->poswork, EVENT_SAMPLING_RATE);
	CPU_ID = !CPU_ID; // toggle cpu to use next time
#endif

	return 0;

 err2:
	input_unregister_device(AMRI5K_ts_input);
	AMRI5K_ts_input = NULL; /* so we dont try to free it below */
 err1:
	input_free_device(AMRI5K_ts_input);
	kfree(priv);
 err0:
	dev_set_drvdata(&spi->dev, NULL);
	return error;
}

static int AMRI5K_remove(struct spi_device *spi)
{
	struct AMRI5K_priv *priv = dev_get_drvdata(&spi->dev);

#ifndef TOUCH_POLLING
	gpio_free(AMRI5K_IRQ_GPIO);
#endif
	input_unregister_device(priv->tdev);
	kfree(priv);

	dev_set_drvdata(&spi->dev, NULL);

	return 0;
}

static struct spi_driver AMRI5K_driver = {
	.driver = {
		.name = TOUCH_NAME,
		.owner = THIS_MODULE,
	},	
	.probe = AMRI5K_probe,
	.remove = AMRI5K_remove,
};

static int __init AMRI5K_init(void)
{
	int ret = 0;

	printk("AMRI5K Driver Module Init\n");
	ret = spi_register_driver(&AMRI5K_driver);
	printk("AMRI5K: %d\n", ret);
	return ret;
}

static void __exit AMRI5K_exit(void)
{
	printk("AMRI5K Driver Module Exit\n");
	spi_unregister_driver(&AMRI5K_driver);
}

MODULE_AUTHOR("AVAGO Technology");
MODULE_DESCRIPTION("AMRI5K TouchController Driver");
MODULE_LICENSE("GPL");

module_init(AMRI5K_init);
module_exit(AMRI5K_exit);
