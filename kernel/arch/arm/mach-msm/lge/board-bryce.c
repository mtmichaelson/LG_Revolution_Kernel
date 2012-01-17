/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/io.h>
#ifdef CONFIG_SPI_QSD
#include <linux/spi/spi.h>
#endif
#include <linux/mfd/pmic8058.h>
#include <linux/mfd/marimba.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/smsc911x.h>
#include <linux/ofn_atlab.h>
#include <linux/power_supply.h>
#include <linux/input/pmic8058-keypad.h>

// BEGIN 0010030 : eundeok.bae@lge.com 2010-10-18
// [KERNEL] Removed functions not used in the board-bryce.c
#if defined(CONFIG_HAPTIC_ISA1200)
#include <linux/i2c/isa1200.h>
#endif
// END 0010030 : eundeok.bae@lge.com 2010-10-18



// BEGIN: 0013023 hyomoon.cho@lge.com 2010-12-25
// ADD 0013023: [PlayReady] first uploading of PlayReady Integration (OpenSource) 

#include <linux/dma-mapping.h>

// END: 0013023 hyomoon.cho@lge.com 2010-12-25

#include <linux/pwm.h>
#include <linux/pmic8058-pwm.h>
#include <linux/i2c/tsc2007.h>
#include <linux/input/kp_flip_switch.h>
#include <linux/leds-pmic8058.h>
#include <linux/input/cy8c_ts.h>
#include <linux/msm_adc.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>

#include <mach/mpp.h>
#include <mach/board.h>
#include <mach/camera.h>
#include <mach/memory.h>
#include <mach/msm_iomap.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#include <mach/msm_spi.h>
#include <mach/qdsp5v2/msm_lpa.h>
#include <mach/dma.h>
#include <linux/android_pmem.h>
#include <linux/input/msm_ts.h>
#include <mach/pmic.h>
#include <mach/rpc_pmapp.h>
#include <mach/qdsp5v2/aux_pcm.h>
#include <mach/qdsp5v2/mi2s.h>
#include <mach/qdsp5v2/audio_dev_ctl.h>
#include <mach/msm_battery.h>
#include <mach/rpc_server_handset.h>
#include <mach/msm_tsif.h>

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	include common structures for bryce-board.
*/
#if CONFIG_MACH_LGE_BRYCE
#include <mach/board-bryce.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/spi.h>
#endif 

#include <asm/mach/mmc.h>
#include <asm/mach/flash.h>
#include <mach/vreg.h>
/* neo.kang@lge.com	10.12.15. S
 * 0012867 : add the hidden reset
 */
#include <mach/board_lge.h>
/* neo.kang@lge.com	10.12.15. E */ 
#include "devices.h"
#include "timer.h"
#include <mach/socinfo.h>
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif
#include "pm.h"
#include "spm.h"
#include <linux/msm_kgsl.h>
#include <mach/dal_axi.h>
#include <mach/msm_serial_hs.h>
#include <mach/msm_reqs.h>
#include <linux/gpio_event.h>

// BEGIN: 0009214 sehyuny.kim@lge.com 2010-09-03
// MOD 0009214: [DIAG] LG Diag feature added in side of android
#include "lg_fw_diag_communication.h"
// END: 0009214 sehyuny.kim@lge.com 2010-09-03


/* hyunjong.do@lge.com  10.08.30
   add proc comm header
 */
#if defined(CONFIG_MACH_LGE_BRYCE) 
#include "proc_comm.h"
#endif
#include <linux/bma150.h>
/* kwangdo.yi 	10.07.06
   add sensor config
   */
#ifdef CONFIG_MACH_LGE_BRYCE
#if defined (LG_HW_REV2)
#define GPIO_ACCEL_INT		81
#define GPIO_COMPASS_RST	78
#define GPIO_COMPASS_IRQ	77
#endif
#if defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
#define GPIO_ACCEL_INT		96
#define GPIO_COMPASS_RST	48
#define GPIO_COMPASS_IRQ	47
#endif
#define GPIO_I2C_SENSOR_SDA	150
#define GPIO_I2C_SENSOR_SCL	149
#define VREG_PERI_26V		"gp4"
#define VREG_SENSOR_IO_18V		"gp7"
#endif

/* neo.kang@lge.com	10.12.15.
 * 0012867 : the pmem definition is moved to board_lge.h */

/* sungmin.shin, sungwoo.cho	10.07.02
	GPIO card detect
*/
#ifdef CONFIG_MACH_LGE_BRYCE
#define SYS_GPIO_SD_DET	55  /* SYS GPIO Number 55 */
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
#define SYS_GPIO_SD_EN_N	97
#endif 
#endif

#define PMIC_GPIO_INT		27
#define PMIC_VREG_WLAN_LEVEL	2900
#define PMIC_GPIO_SD_DET	36
#define PMIC_GPIO_SDC4_EN	17  /* PMIC GPIO Number 18 */
#define PMIC_GPIO_HDMI_5V_EN	39  /* PMIC GPIO Number 40 */
#define ADV7520_I2C_ADDR	0x39

#define FPGA_SDCC_STATUS       0x8E0001A8

#define FPGA_OPTNAV_GPIO_ADDR	0x8E000026
#define OPTNAV_I2C_SLAVE_ADDR	(0xB0 >> 1)
#define OPTNAV_IRQ		20
#define OPTNAV_CHIP_SELECT	19

/* Macros assume PMIC GPIOs start at 0 */
#define PM8058_GPIO_PM_TO_SYS(pm_gpio)     (pm_gpio + NR_GPIO_IRQS)
#define PM8058_GPIO_SYS_TO_PM(sys_gpio)    (sys_gpio - NR_GPIO_IRQS)

#define PMIC_GPIO_HAP_ENABLE   16  /* PMIC GPIO Number 17 */

#define PMIC_GPIO_WLAN_EXT_POR  22 /* PMIC GPIO NUMBER 23 */

#define BMA150_GPIO_INT 1
#define HAP_LVL_SHFT_MSM_GPIO 24

#define	PM_FLIP_MPP 5 /* PMIC MPP 06 */

/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.08.09
	For LTE SDIO Driver
*/
#ifdef  CONFIG_MACH_LGE_BRYCE
#ifdef CONFIG_LTE
#define GPIO_LTE_SDIO_PWR 154
#define GPIO_LTE_SDIO_RESET 157
//#define GPIO_LTE_SDIO_DETECT 3 /* test SDIO detect */
#define GPIO_LTE_SDIO_DATA_0 43
#define GPIO_LTE_SDIO_DATA_1 42
#define GPIO_LTE_SDIO_DATA_2 41
#define GPIO_LTE_SDIO_DATA_3 40
#define GPIO_LTE_SDIO_CMD 39
#define GPIO_LTE_SDIO_CLK 38

/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */
#define GPIO_L2K_LTE_WAKEUP 153
#define GPIO_L2K_LTE_STATUS 32
#define GPIO_L2K_HOST_WAKEUP 18
#define GPIO_L2K_HOST_STATUS 123
/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   			

#endif //CONFIG_LTE
#endif //CONFIG_MACH_LGE_BRYCE
/* BEGIN:0010882        ehgrace.kim@lge.com     2010.11.15*/
/* MOD: add the call mode */
#define LGE_AUDIO_PATH 1

/* ey.cho@lge.com    11.02.27
   START 0017240: [Touch] Changed initialise code that power on after power off  */
extern int touch_power(int enable);
/*   END 0017240: [Touch] Changed initialise code that power on after power off  */

static int pm8058_gpios_init(void)
{
	int rc;
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	struct pm8058_gpio sdcc_det = {
		.direction      = PM_GPIO_DIR_IN,
		.pull           = PM_GPIO_PULL_UP_1P5,
		.vin_sel        = 2,
		.function       = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol    = 0,
	};
#endif
	struct pm8058_gpio hdmi_5V_en = {
		.direction      = PM_GPIO_DIR_OUT,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel        = PM_GPIO_VIN_VPH,
		.function       = PM_GPIO_FUNC_NORMAL,
	};

	/* neo.kang@lge.com	10.12.15.
	 * 0012867 : delete the code related with fluid */

	struct pm8058_gpio gpio23 = {
			.direction      = PM_GPIO_DIR_OUT,
			.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = 2,
			.out_strength   = PM_GPIO_STRENGTH_LOW,
			.function       = PM_GPIO_FUNC_NORMAL,
	};
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	/* neo.kang@lge.com	10.12.15.
	 * 0012867 : delete the code related with fluid */

	rc = pm8058_gpio_config(PMIC_GPIO_SD_DET - 1, &sdcc_det);
	if (rc) {
		pr_err("%s PMIC_GPIO_SD_DET config failed\n", __func__);
		return rc;
	}
#endif

	rc = pm8058_gpio_config(PMIC_GPIO_HDMI_5V_EN, &hdmi_5V_en);
	if (rc) {
		pr_err("%s PMIC_GPIO_HDMI_5V_EN config failed\n", __func__);
		return rc;
	}
	/* Deassert GPIO#23 (source for Ext_POR on WLAN-Volans) */
	rc = pm8058_gpio_config(PMIC_GPIO_WLAN_EXT_POR, &gpio23);
	if (rc) {
		pr_err("%s PMIC_GPIO_WLAN_EXT_POR config failed\n", __func__);
		return rc;
	}
	rc = gpio_request(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_HDMI_5V_EN),
		"hdmi_5V_en");
	if (rc) {
		pr_err("%s PMIC_GPIO_HDMI_5V_EN gpio_request failed\n",
			__func__);
		return rc;
	}

	/* neo.kang@lge.com	10.12.15.
	 * 0012867 : delete the code related with fluid */

	return 0;
}

static int pm8058_pwm_config(struct pwm_device *pwm, int ch, int on)
{
	struct pm8058_gpio pwm_gpio_config = {
		.direction      = PM_GPIO_DIR_OUT,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 0,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel        = PM_GPIO_VIN_S3,
		.out_strength   = PM_GPIO_STRENGTH_HIGH,
		.function       = PM_GPIO_FUNC_2,
	};
	int	rc = -EINVAL;
	int	id, mode, max_mA;

	id = mode = max_mA = 0;
	switch (ch) {
	case 0:
	case 1:
	case 2:
		if (on) {
			id = 24 + ch;
			rc = pm8058_gpio_config(id - 1, &pwm_gpio_config);
			if (rc)
				pr_err("%s: pm8058_gpio_config(%d): rc=%d\n",
				       __func__, id, rc);
		}
		break;

	case 3:
		id = PM_PWM_LED_KPD;
		mode = PM_PWM_CONF_DTEST3;
		max_mA = 200;
		break;

	case 4:
		id = PM_PWM_LED_0;
		mode = PM_PWM_CONF_PWM1;
		max_mA = 40;
		break;

	case 5:
		id = PM_PWM_LED_2;
		mode = PM_PWM_CONF_PWM2;
		max_mA = 40;
		break;

	case 6:
		id = PM_PWM_LED_FLASH;
		mode = PM_PWM_CONF_DTEST3;
		max_mA = 200;
		break;

	default:
		break;
	}

	if (ch >= 3 && ch <= 6) {
		if (!on) {
			mode = PM_PWM_CONF_NONE;
			max_mA = 0;
		}
		rc = pm8058_pwm_config_led(pwm, id, mode, max_mA);
		if (rc)
			pr_err("%s: pm8058_pwm_config_led(ch=%d): rc=%d\n",
			       __func__, ch, rc);
	}

	return rc;
}

static int pm8058_pwm_enable(struct pwm_device *pwm, int ch, int on)
{
	int	rc;

	switch (ch) {
	case 7:
		rc = pm8058_pwm_set_dtest(pwm, on);
		if (rc)
			pr_err("%s: pwm_set_dtest(%d): rc=%d\n",
			       __func__, on, rc);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static struct resource resources_keypad[] = {
	{
		.start	= PM8058_KEYPAD_IRQ(PMIC8058_IRQ_BASE),
		.end	= PM8058_KEYPAD_IRQ(PMIC8058_IRQ_BASE),
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= PM8058_KEYSTUCK_IRQ(PMIC8058_IRQ_BASE),
		.end	= PM8058_KEYSTUCK_IRQ(PMIC8058_IRQ_BASE),
		.flags	= IORESOURCE_IRQ,
	},
};
/* sungmin.shin 10.07.14
	keypad mapping
*/
#if CONFIG_MACH_LGE_BRYCE
#ifdef LG_HW_REV1
static const unsigned int bryce_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(1, 0, KEY_VOLUMEDOWN),
	//KEY(2, 0, KEY_SEND),
	KEY(2, 0, KEY_5), //TEMP. FIX ME
	KEY(3, 0, KEY_END),	
	KEY(4, 0, KEY_BACK), //CLEAR
	KEY(5, 0, KEY_HOME),
};

static struct pmic8058_keypad_data bryce_keypad_data = {
	.input_name		= "surf_keypad",
	.input_phys_device	= "surf_keypad/input0",
	.num_rows		= 12,
	.num_cols		= 8,
	.rows_gpio_start	= 8,
	.cols_gpio_start	= 0,
	.keymap_size		= ARRAY_SIZE(bryce_keymap),
	.keymap			= bryce_keymap,
	.debounce_ms		= {8, 10},
	.scan_delay_ms		= 32,
	.row_hold_ns		= 91500,
	.wakeup			= 1,
};
#endif 
#endif

/*ey.cho	2010.06.25
	for touch keypad in EVB2 
*/
#if defined(CONFIG_MACH_LGE_BRYCE)
#ifdef LG_HW_REV2
static struct tskey_platform_data tskey_data = {
	.attn = 180,
};

static struct i2c_board_info tskey_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("so240001", 0x2c),
		.type = "so240001",
		.platform_data = &tskey_data,
	},
};

static struct gpio_i2c_pin tskey_i2c_pin[] = {
	[0] = {
		.sda_pin        = 33,
		.scl_pin        = 32,
		.irq_pin     		= 180,
	},
};

static struct i2c_gpio_platform_data tskey_i2c_pdata = {
	.sda_is_open_drain      = 0,
	.scl_is_open_drain      = 0,
	.udelay         = 2,
};

static struct platform_device tskey_i2c_device = {
	.id = 10,
	.name = "i2c-gpio",
	.dev.platform_data = &tskey_i2c_pdata,
};
#endif
#endif

/* CONFIG_MACH_LGE_BRYCE	ehgrace.kim 10.05.03
	add amp and mic_sel
*/

/* sungmin.shin	10.07.29
	assign gpio number into NC temporarily.
	TODO. assign right gpio number.
*/

/* CONFIG_MACH_LGE_BRYCE	ehgrace.kim 10.08.18
	add CAM_MIC_EN(GPIO#86) which should set to low for no-call mode
*/

#if defined(LG_HW_REV1) 
#define GPIO_AUDIO 94
#define GPIO_AUDIO2 94
/* kenneth.kang 2010-11-17 [Start] [Mod] modify HW Revision build info */
#elif defined(LG_HW_REV2) 
#define GPIO_AUDIO 80
#define GPIO_AUDIO2 80
/*BEGIN: 0011460 daeok.kim@lge.com 2010-11-27 */
/*MOD 0011460: [LTE] Since Rev.D, revision check method is modified by using PMIC voltage level check */
#elif defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5)
#define GPIO_AUDIO 86
#define GPIO_AUDIO2 80
#else //defined(LG_HW_REV6) || defined(LG_HW_REV7)
#define GPIO_AUDIO 86
#define GPIO_AUDIO2 83 /*Do not use GPIO 80 since Rev.D*/
#endif 
/* kenneth.kang 2010-11-17 [END] */

/*BEGIN: 0010961 daeok.kim@lge.com 2010-11-16 */
/*MOD 0010961: [LTE] GPIO (L2K_1V2_CORE_EN) is changed: 83 ->80 */ 
//#if defined(LG_HW_REV7)
	//#define GPIO_AUDIO2 83
//#endif 
/*END: 0010961 daeok.kim@lge.com 2010-11-16 */
/*END: 0011460 daeok.kim@lge.com 2010-11-27 */

/* CONFIG_MACH_LGE_BRYCE	ehgrace.kim 10.05.03
	mic_sel
*/
#if defined (CONFIG_MACH_LGE_BRYCE)
static uint32_t audio_pamp_gpio_config =
   GPIO_CFG(GPIO_AUDIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA);
#else
static uint32_t audio_pamp_gpio_config =
   GPIO_CFG(82, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA);
#endif 

#if defined (CONFIG_MACH_LGE_BRYCE)
void lge_snddev_spk_amp_on(void)
{
	set_amp_gain(4); /* enable spkr amp */
	pr_info("%s: set the amplifier\n", __func__);
}

void lge_snddev_hs_amp_on(void)
{	
	set_amp_gain(2); /* enable hs amp */
	pr_info("%s: set the amplifier\n", __func__);
}
//For sonification stream need to turn on both headset & speaker.  
// ++ kiran.kanneganti@lge.com
/* BEGIN:0009748	ehgrace.kim@lge.com	2010.10.07*/
/* MOD:	modifiy the amp for sonification mode for subsystem audio calibration */
void lge_snddev_spk_hs_amp_on(void)
{
	set_amp_gain(3); /* enable hs & spkr amp */
	pr_info("%s: set the amplifier\n", __func__);
}
/* END:0009748	ehgrace.kim@lge.com	2010.10.07*/
// -- kiran.kanneganti@lge.com
#if LGE_AUDIO_PATH
/* BEGIN:0010882        ehgrace.kim@lge.com     2010.11.15*/
/* MOD: add the call mode */
void lge_snddev_spk_phone_amp_on(void)
{
	set_amp_gain(7); /* enable spkr amp */
	pr_info("%s: set the amplifier\n", __func__);
}

void lge_snddev_hs_phone_amp_on(void)
{	
	set_amp_gain(6); /* enable hs amp */
	pr_info("%s: set the amplifier\n", __func__);
} 
/* END:0010882        ehgrace.kim@lge.com     2010.11.15*/
#endif
void lge_snddev_amp_off(void)
{
	set_amp_gain(5); /* disable amp */
	pr_info("%s: set the amplifier\n", __func__);
}
#endif

static struct pm8058_pwm_pdata pm8058_pwm_data = {
	.config		= pm8058_pwm_config,
	.enable		= pm8058_pwm_enable,
};

/* Put sub devices with fixed location first in sub_devices array */
#define	PM8058_SUBDEV_KPD	0
#define	PM8058_SUBDEV_LED	1

static struct pm8058_gpio_platform_data pm8058_gpio_data = {
	.gpio_base	= PM8058_GPIO_PM_TO_SYS(0),
	.irq_base	= PM8058_GPIO_IRQ(PMIC8058_IRQ_BASE, 0),
	.init		= pm8058_gpios_init,
};

static struct pm8058_gpio_platform_data pm8058_mpp_data = {
	.gpio_base	= PM8058_GPIO_PM_TO_SYS(PM8058_GPIOS),
	.irq_base	= PM8058_MPP_IRQ(PMIC8058_IRQ_BASE, 0),
};

static struct pmic8058_led pmic8058_ffa_leds[] = {
	[0] = {
		.name		= "keyboard-backlight",
		.max_brightness = 15,
		.id		= PMIC8058_ID_LED_KB_LIGHT,
	},
};

static struct pmic8058_leds_platform_data pm8058_ffa_leds_data = {
	.num_leds = ARRAY_SIZE(pmic8058_ffa_leds),
	.leds	= pmic8058_ffa_leds,
};

/* jihye.ahn	10.08.18
	keyboard-backlight & RGB indicator
*/
static struct pmic8058_led pmic8058_surf_leds[] = {
	[0] = {
		.name		= "button-backlight",
		.max_brightness = 15,
		.id		= PMIC8058_ID_LED_KB_LIGHT,
	},
	[1] = {
		.name		= "red",
		.max_brightness = 20,
		.id		= PMIC8058_ID_LED_0,
	},
	[2] = {
		.name		= "green",
		.max_brightness = 20,
		.id		= PMIC8058_ID_LED_1,
	},
	[3] = {
		.name		= "blue",
		.max_brightness = 20,
		.id 	= PMIC8058_ID_LED_2,
		},

};

static struct mfd_cell pm8058_subdevs[] = {
	{	.name = "pm8058-keypad",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(resources_keypad),
		.resources	= resources_keypad,
	},
	{	.name = "pm8058-led",
		.id		= -1,
	},
	{	.name = "pm8058-gpio",
		.id		= -1,
		.platform_data	= &pm8058_gpio_data,
		.data_size	= sizeof(pm8058_gpio_data),
	},
	{	.name = "pm8058-mpp",
		.id		= -1,
		.platform_data	= &pm8058_mpp_data,
		.data_size	= sizeof(pm8058_mpp_data),
	},
	{	.name = "pm8058-pwm",
		.id		= -1,
		.platform_data	= &pm8058_pwm_data,
		.data_size	= sizeof(pm8058_pwm_data),
	},
	{	.name = "pm8058-nfc",
		.id		= -1,
	},
	{	.name = "pm8058-upl",
		.id		= -1,
	},
};

static struct pmic8058_leds_platform_data pm8058_surf_leds_data = {
	.num_leds = ARRAY_SIZE(pmic8058_surf_leds),
	.leds	= pmic8058_surf_leds,
};

static struct pm8058_platform_data pm8058_7x30_data = {
	.irq_base = PMIC8058_IRQ_BASE,

	.num_subdevs = ARRAY_SIZE(pm8058_subdevs),
	.sub_devices = pm8058_subdevs,
	.irq_trigger_flags = IRQF_TRIGGER_LOW, //GB Change
};

static struct i2c_board_info pm8058_boardinfo[] __initdata = {
	{
		I2C_BOARD_INFO("pm8058-core", 0x55), //GB Change
		.irq = MSM_GPIO_TO_INT(PMIC_GPIO_INT),
		.platform_data = &pm8058_7x30_data,
	},
};


#ifdef CONFIG_MSM_GEMINI
static struct resource msm_gemini_resources[] = {
	{
		.start  = 0xA3A00000,
		.end    = 0xA3A00000 + 0x0150 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = INT_JPEG,
		.end    = INT_JPEG,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device msm_gemini_device = {
	.name           = "msm_gemini",
	.resource       = msm_gemini_resources,
	.num_resources  = ARRAY_SIZE(msm_gemini_resources),
};
#endif

#ifdef CONFIG_MSM_VPE
static struct resource msm_vpe_resources[] = {
	{
		.start	= 0xAD200000,
		.end	= 0xAD200000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_VPE,
		.end	= INT_VPE,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_vpe_device = {
       .name = "msm_vpe",
       .id   = 0,
       .num_resources = ARRAY_SIZE(msm_vpe_resources),
       .resource = msm_vpe_resources,
};
#endif

#ifdef CONFIG_MSM7KV2_AUDIO

static int __init snddev_poweramp_gpio_init(void)
{
	int rc;

	pr_info("snddev_poweramp_gpio_init \n");
	rc = gpio_tlmm_config(audio_pamp_gpio_config, GPIO_CFG_ENABLE);
	if (rc) {
		printk(KERN_ERR
			"%s: gpio_tlmm_config(%#x)=%d\n",
			__func__, audio_pamp_gpio_config, rc);
	}
	return rc;
}

/* daniel.kang@lge.com ++ : GPIO_AUDIO is CAM_MIC_EN pin */
/* if CAM_MIC_EN = 1, sub mic goes to audience. */
/* if CAM_MIC_EN = 0, sub mic goes to QTR8650.  */
#if defined (CONFIG_MACH_LGE_BRYCE)
/* daniel.kang@lge.com ++ Nov 17 // 0011018: Remove Audience Code */
#if defined(LG_HW_REV4) || defined(LG_HW_REV5)
void lge_snddev_MSM_mic_route_config(void)
{
	int rc = 0;
	pr_debug("%s()\n", __func__);
	rc = gpio_tlmm_config(audio_pamp_gpio_config,GPIO_CFG_ENABLE);
	if (rc) 
	{
		printk(KERN_ERR"%s: gpio_tlmm_config(%#x)=%d\n",__func__, audio_pamp_gpio_config, rc);
	} 
	else
		gpio_set_value(GPIO_AUDIO, 0);	/* To MSM Mic*/
	pr_info("%s: set the amplifier :%d\n", __func__, GPIO_AUDIO);

}
#endif	// #if defined(LG_HW_REV4) || defined(LG_HW_REV5)
/* daniel.kang@lge.com -- Nov 17 // 0011018: Remove Audience Code */
#endif
/* daniel.kang@lge.com -- : GPIO_AUDIO is CAM_MIC_EN pin */


void msm_snddev_tx_route_config(void)
{
	pr_debug("%s()\n", __func__);

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
}

void msm_snddev_tx_route_deconfig(void)
{
	pr_debug("%s()\n", __func__);

/* neo.kang@lge.com	10.12.15.
 * 0012867 :delete the code related with fluid */
}

void msm_snddev_poweramp_on(void)
{
	//gpio_set_value(82, 1);	/* enable spkr poweramp */
	pr_info("%s: power on amplifier\n", __func__);
}

void msm_snddev_poweramp_off(void)
{
	//gpio_set_value(82, 0);	/* disable spkr poweramp */
	pr_info("%s: power off amplifier\n", __func__);
}

static struct vreg *snddev_vreg_ncp, *snddev_vreg_gp4;

void msm_snddev_hsed_voltage_on(void)
{
	int rc;

	snddev_vreg_gp4 = vreg_get(NULL, "gp4");
	if (IS_ERR(snddev_vreg_gp4)) {
		pr_err("%s: vreg_get(%s) failed (%ld)\n",
		__func__, "gp4", PTR_ERR(snddev_vreg_gp4));
		return;
	}
	rc = vreg_enable(snddev_vreg_gp4);
	if (rc)
		pr_err("%s: vreg_enable(gp4) failed (%d)\n", __func__, rc);

	snddev_vreg_ncp = vreg_get(NULL, "ncp");
	if (IS_ERR(snddev_vreg_ncp)) {
		pr_err("%s: vreg_get(%s) failed (%ld)\n",
		__func__, "ncp", PTR_ERR(snddev_vreg_ncp));
		return;
	}
	rc = vreg_enable(snddev_vreg_ncp);
	if (rc)
		pr_err("%s: vreg_enable(ncp) failed (%d)\n", __func__, rc);
}

void msm_snddev_hsed_voltage_off(void)
{
	int rc;

	if (IS_ERR(snddev_vreg_ncp)) {
		pr_err("%s: vreg_get(%s) failed (%ld)\n",
		__func__, "ncp", PTR_ERR(snddev_vreg_ncp));
		return;
	}
	rc = vreg_disable(snddev_vreg_ncp);
	if (rc)
		pr_err("%s: vreg_disable(ncp) failed (%d)\n", __func__, rc);
	vreg_put(snddev_vreg_ncp);

	if (IS_ERR(snddev_vreg_gp4)) {
		pr_err("%s: vreg_get(%s) failed (%ld)\n",
		__func__, "gp4", PTR_ERR(snddev_vreg_gp4));
		return;
	}
	rc = vreg_disable(snddev_vreg_gp4);
	if (rc)
		pr_err("%s: vreg_disable(gp4) failed (%d)\n", __func__, rc);

	vreg_put(snddev_vreg_gp4);

}

static unsigned aux_pcm_gpio_on[] = {
	GPIO_CFG(138, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_DOUT */
	GPIO_CFG(139, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_DIN  */
	GPIO_CFG(140, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_SYNC */
	GPIO_CFG(141, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_CLK  */
};

static int __init aux_pcm_gpio_init(void)
{
	int pin, rc;

	pr_info("aux_pcm_gpio_init \n");
	for (pin = 0; pin < ARRAY_SIZE(aux_pcm_gpio_on); pin++) {
		rc = gpio_tlmm_config(aux_pcm_gpio_on[pin],
					GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR
				"%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, aux_pcm_gpio_on[pin], rc);
		}
	}
	return rc;
}

static struct msm_gpio mi2s_clk_gpios[] = {
	{ GPIO_CFG(145, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_SCLK"},
	{ GPIO_CFG(144, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_WS"},

/* BEGIN:0018829        ehgrace.kim@lge.com     2011.04.15*/
/* MOD: remove HDMI MCLK configuration because it is conflicted with VT_CAM_PWDN */
#if 0
	{ GPIO_CFG(120, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_MCLK_A"},
#endif
/* END:0018829        ehgrace.kim@lge.com     2011.04.15*/
};

static struct msm_gpio mi2s_rx_data_lines_gpios[] = {
	{ GPIO_CFG(121, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD0_A"},
	{ GPIO_CFG(122, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD1_A"},
	{ GPIO_CFG(123, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD2_A"},
	{ GPIO_CFG(146, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD3"},
};

static struct msm_gpio mi2s_tx_data_lines_gpios[] = {
	{ GPIO_CFG(146, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD3"},
};

int mi2s_config_clk_gpio(void)
{
	int rc = 0;

	rc = msm_gpios_request_enable(mi2s_clk_gpios,
			ARRAY_SIZE(mi2s_clk_gpios));
	if (rc) {
		pr_err("%s: enable mi2s clk gpios  failed\n",
					__func__);
		return rc;
	}
	return 0;
}

int  mi2s_unconfig_data_gpio(u32 direction, u8 sd_line_mask)
{
	int i, rc = 0;
	sd_line_mask &= MI2S_SD_LINE_MASK;

	switch (direction) {
	case DIR_TX:
		msm_gpios_disable_free(mi2s_tx_data_lines_gpios, 1);
		break;
	case DIR_RX:
		i = 0;
		while (sd_line_mask) {
			if (sd_line_mask & 0x1)
				msm_gpios_disable_free(
					mi2s_rx_data_lines_gpios + i , 1);
			sd_line_mask = sd_line_mask >> 1;
			i++;
		}
		break;
	default:
		pr_err("%s: Invaild direction  direction = %u\n",
						__func__, direction);
		rc = -EINVAL;
		break;
	}
	return rc;
}

int mi2s_config_data_gpio(u32 direction, u8 sd_line_mask)
{
	int i , rc = 0;
	u8 sd_config_done_mask = 0;

	sd_line_mask &= MI2S_SD_LINE_MASK;

	switch (direction) {
	case DIR_TX:
		if ((sd_line_mask & MI2S_SD_0) || (sd_line_mask & MI2S_SD_1) ||
		   (sd_line_mask & MI2S_SD_2) || !(sd_line_mask & MI2S_SD_3)) {
			pr_err("%s: can not use SD0 or SD1 or SD2 for TX"
				".only can use SD3. sd_line_mask = 0x%x\n",
				__func__ , sd_line_mask);
			rc = -EINVAL;
		} else {
			rc = msm_gpios_request_enable(mi2s_tx_data_lines_gpios,
							 1);
			if (rc)
				pr_err("%s: enable mi2s gpios for TX failed\n",
					   __func__);
		}
		break;
	case DIR_RX:
		i = 0;
		while (sd_line_mask && (rc == 0)) {
			if (sd_line_mask & 0x1) {
				rc = msm_gpios_request_enable(
					mi2s_rx_data_lines_gpios + i , 1);
				if (rc) {
					pr_err("%s: enable mi2s gpios for"
					 "RX failed.  SD line = %s\n",
					 __func__,
					 (mi2s_rx_data_lines_gpios + i)->label);
					mi2s_unconfig_data_gpio(DIR_RX,
						sd_config_done_mask);
				} else
					sd_config_done_mask |= (1 << i);
			}
			sd_line_mask = sd_line_mask >> 1;
			i++;
		}
		break;
	default:
		pr_err("%s: Invaild direction  direction = %u\n",
			__func__, direction);
		rc = -EINVAL;
		break;
	}
	return rc;
}

int mi2s_unconfig_clk_gpio(void)
{
	msm_gpios_disable_free(mi2s_clk_gpios, ARRAY_SIZE(mi2s_clk_gpios));
	return 0;
}

#endif /* CONFIG_MSM7KV2_AUDIO */

static int __init buses_init(void)
{
	if (gpio_tlmm_config(GPIO_CFG(PMIC_GPIO_INT, 1, GPIO_CFG_INPUT,
				  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE))
		pr_err("%s: gpio_tlmm_config (gpio=%d) failed\n",
		       __func__, PMIC_GPIO_INT);

/* sungmin.shin 10.07.14
	keypad register
*/
#if CONFIG_MACH_LGE_BRYCE
#ifdef LG_HW_REV1
	pm8058_7x30_data.sub_devices[PM8058_SUBDEV_KPD].platform_data
		= &bryce_keypad_data;
	pm8058_7x30_data.sub_devices[PM8058_SUBDEV_KPD].data_size
		= sizeof(bryce_keypad_data);
#endif
#endif

	i2c_register_board_info(6 /* I2C_SSBI ID */, pm8058_boardinfo,
				ARRAY_SIZE(pm8058_boardinfo));

	return 0;
}

#define TIMPANI_RESET_GPIO	1

struct bahama_config_register{
	u8 reg;
	u8 value;
	u8 mask;
};

enum version{
	VER_1_0,
	VER_2_0,
	VER_UNSUPPORTED = 0xFF
};


static struct vreg *vreg_marimba_1;
static struct vreg *vreg_marimba_2;
static struct vreg *vreg_marimba_3;

static struct msm_gpio timpani_reset_gpio_cfg[] = {
{ GPIO_CFG(TIMPANI_RESET_GPIO, 0, GPIO_CFG_OUTPUT,
	GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "timpani_reset"} };

static u8 read_bahama_ver(void)
{
	int rc;
	struct marimba config = { .mod_id = SLAVE_ID_BAHAMA };
	u8 bahama_version;

	rc = marimba_read_bit_mask(&config, 0x00,  &bahama_version, 1, 0x1F);
	if (rc < 0) {
		printk(KERN_ERR
			 "%s: version read failed: %d\n",
			__func__, rc);
			return rc;
	} else {
		printk(KERN_INFO
		"%s: version read got: 0x%x\n",
		__func__, bahama_version);
	}

	switch (bahama_version) {
	case 0x08: /* varient of bahama v1 */
	case 0x10:
	case 0x00:
		return VER_1_0;
	case 0x09: /* variant of bahama v2 */
		return VER_2_0;
	default:
		return VER_UNSUPPORTED;
	}
}

static int config_timpani_reset(void)
{
	int rc;

	rc = msm_gpios_request_enable(timpani_reset_gpio_cfg,
				ARRAY_SIZE(timpani_reset_gpio_cfg));
	if (rc < 0) {
		printk(KERN_ERR
			"%s: msm_gpios_request_enable failed (%d)\n",
				__func__, rc);
	}
	return rc;
}

static unsigned int msm_timpani_setup_power(void)
{
	int rc;

	rc = config_timpani_reset();
	if (rc < 0)
		goto out;

	rc = vreg_enable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
					__func__, rc);
		goto out;
	}
	rc = vreg_enable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
					__func__, rc);
		goto fail_disable_vreg_marimba_1;
	}

	rc = gpio_direction_output(TIMPANI_RESET_GPIO, 1);
	if (rc < 0) {
		printk(KERN_ERR
			"%s: gpio_direction_output failed (%d)\n",
				__func__, rc);
		msm_gpios_free(timpani_reset_gpio_cfg,
				ARRAY_SIZE(timpani_reset_gpio_cfg));
		vreg_disable(vreg_marimba_2);
	} else
		goto out;


fail_disable_vreg_marimba_1:
	vreg_disable(vreg_marimba_1);

out:
	return rc;
};

static void msm_timpani_shutdown_power(void)
{
	int rc;

	rc = vreg_disable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
	}
	rc = vreg_disable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
	}

	rc = gpio_direction_output(TIMPANI_RESET_GPIO, 0);
	if (rc < 0) {
		printk(KERN_ERR
			"%s: gpio_direction_output failed (%d)\n",
				__func__, rc);
	}

	msm_gpios_free(timpani_reset_gpio_cfg,
				   ARRAY_SIZE(timpani_reset_gpio_cfg));
};

static unsigned int msm_bahama_core_config(int type)
{
	int rc = 0;

	if (type == BAHAMA_ID) {

		int i;
		struct marimba config = { .mod_id = SLAVE_ID_BAHAMA };

		const struct bahama_config_register v20_init[] = {
			/* reg, value, mask */
			{ 0xF4, 0x84, 0xFF }, /* AREG */
			{ 0xF0, 0x04, 0xFF } /* DREG */
		};

		if (read_bahama_ver() == VER_2_0) {
			for (i = 0; i < ARRAY_SIZE(v20_init); i++) {
				u8 value = v20_init[i].value;
				rc = marimba_write_bit_mask(&config,
					v20_init[i].reg,
					&value,
					sizeof(v20_init[i].value),
					v20_init[i].mask);
				if (rc < 0) {
					printk(KERN_ERR
						"%s: reg %d write failed: %d\n",
						__func__, v20_init[i].reg, rc);
					return rc;
				}
				printk(KERN_INFO "%s: reg 0x%02x value 0x%02x"
					" mask 0x%02x\n",
					__func__, v20_init[i].reg,
					v20_init[i].value, v20_init[i].mask);
			}
		}
	}
	printk(KERN_INFO "core type: %d\n", type);

	return rc;
}

static unsigned int msm_bahama_setup_power(void)
{
	int rc;

	rc = vreg_enable(vreg_marimba_3);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
				__func__, rc);
	}

	return rc;
};

static unsigned int msm_bahama_shutdown_power(int value)
{
	int rc = 0;

	if (value != BAHAMA_ID) {
		rc = vreg_disable(vreg_marimba_3);
		if (rc) {
			printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
		}
	}

	return rc;
};

static struct msm_gpio marimba_svlte_config_clock[] = {
	{ GPIO_CFG(34, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		"MARIMBA_SVLTE_CLOCK_ENABLE" },
};

static unsigned int msm_marimba_gpio_config_svlte(int gpio_cfg_marimba)
{
	if (machine_is_msm8x55_svlte_surf() ||
		machine_is_msm8x55_svlte_ffa()) {
		if (gpio_cfg_marimba)
			gpio_set_value(GPIO_PIN
				(marimba_svlte_config_clock->gpio_cfg), 1);
		else
			gpio_set_value(GPIO_PIN
				(marimba_svlte_config_clock->gpio_cfg), 0);
	}

	return 0;
};

static unsigned int msm_marimba_setup_power(void)
{
	int rc;

	rc = vreg_enable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		goto out;
	}
	rc = vreg_enable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		goto out;
	}

	if (machine_is_msm8x55_svlte_surf() || machine_is_msm8x55_svlte_ffa()) {
		rc = msm_gpios_request_enable(marimba_svlte_config_clock,
				ARRAY_SIZE(marimba_svlte_config_clock));
		if (rc < 0) {
			printk(KERN_ERR
				"%s: msm_gpios_request_enable failed (%d)\n",
					__func__, rc);
			return rc;
		}

		rc = gpio_direction_output(GPIO_PIN
			(marimba_svlte_config_clock->gpio_cfg), 0);
		if (rc < 0) {
			printk(KERN_ERR
				"%s: gpio_direction_output failed (%d)\n",
					__func__, rc);
			return rc;
		}
	}

out:
	return rc;
};

static void msm_marimba_shutdown_power(void)
{
	int rc;

	rc = vreg_disable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
	}
	rc = vreg_disable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
};

static int bahama_present(void)
{
	int id;
	switch (id = adie_get_detected_connectivity_type()) {
	case BAHAMA_ID:
		return 1;

	case MARIMBA_ID:
		return 0;

	case TIMPANI_ID:
	default:
	printk(KERN_ERR "%s: unexpected adie connectivity type: %d\n",
			__func__, id);
	return -ENODEV;
	}
}

struct vreg *fm_regulator;
static int fm_radio_setup(struct marimba_fm_platform_data *pdata)
{
	int rc;
	uint32_t irqcfg;
	const char *id = "FMPW";

	int bahama_not_marimba = bahama_present();

	if (bahama_not_marimba == -1) {
		printk(KERN_WARNING "%s: bahama_present: %d\n",
				__func__, bahama_not_marimba);
		return -ENODEV;
	}
	if (bahama_not_marimba)
		fm_regulator = vreg_get(NULL, "s3");
	else
		fm_regulator = vreg_get(NULL, "s2");

	if (IS_ERR(fm_regulator)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(fm_regulator));
		return -1;
	}
	if (!bahama_not_marimba) {

		rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);

		if (rc < 0) {
			printk(KERN_ERR "%s: voltage level vote failed (%d)\n",
				__func__, rc);
			return rc;
		}
	}
	rc = vreg_enable(fm_regulator);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
					__func__, rc);
		return rc;
	}

	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_ON);
	if (rc < 0) {
		printk(KERN_ERR "%s: clock vote failed (%d)\n",
			__func__, rc);
		goto fm_clock_vote_fail;
	}
	/*Request the Clock Using GPIO34/AP2MDM_MRMBCK_EN in case
	of svlte*/
	if (machine_is_msm8x55_svlte_surf() ||
			machine_is_msm8x55_svlte_ffa())	{
		rc = marimba_gpio_config(1);
		if (rc < 0)
			printk(KERN_ERR "%s: clock enable for svlte : %d\n",
						__func__, rc);
	}
	irqcfg = GPIO_CFG(147, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
					GPIO_CFG_2MA);
	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc) {
		printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, irqcfg, rc);
		rc = -EIO;
		goto fm_gpio_config_fail;

	}
	return 0;
fm_gpio_config_fail:
	pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
				  PMAPP_CLOCK_VOTE_OFF);
fm_clock_vote_fail:
	vreg_disable(fm_regulator);
	return rc;

};

static void fm_radio_shutdown(struct marimba_fm_platform_data *pdata)
{
	int rc;
	const char *id = "FMPW";
	uint32_t irqcfg = GPIO_CFG(147, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_2MA);

	int bahama_not_marimba = bahama_present();
	if (bahama_not_marimba == -1) {
		printk(KERN_WARNING "%s: bahama_present: %d\n",
			__func__, bahama_not_marimba);
		return;
	}

	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc) {
		printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, irqcfg, rc);
	}
	if (fm_regulator != NULL) {
		rc = vreg_disable(fm_regulator);

		if (rc) {
			printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
		}
		fm_regulator = NULL;
	}
	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_OFF);
	if (rc < 0)
		printk(KERN_ERR "%s: clock_vote return val: %d\n",
						__func__, rc);

	/*Disable the Clock Using GPIO34/AP2MDM_MRMBCK_EN in case
	of svlte*/
	if (machine_is_msm8x55_svlte_surf() ||
			machine_is_msm8x55_svlte_ffa())	{
		rc = marimba_gpio_config(0);
		if (rc < 0)
			printk(KERN_ERR "%s: clock disable for svlte : %d\n",
						__func__, rc);
	}


	if (!bahama_not_marimba)	{
		rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);

		if (rc < 0)
			printk(KERN_ERR "%s: vreg level vote return val: %d\n",
						__func__, rc);
	}
}

static struct marimba_fm_platform_data marimba_fm_pdata = {
	.fm_setup =  fm_radio_setup,
	.fm_shutdown = fm_radio_shutdown,
	.irq = MSM_GPIO_TO_INT(147),
	.vreg_s2 = NULL,
	.vreg_xo_out = NULL,
};


/* Slave id address for FM/CDC/QMEMBIST
 * Values can be programmed using Marimba slave id 0
 * should there be a conflict with other I2C devices
 * */
#define MARIMBA_SLAVE_ID_FM_ADDR	0x2A
#define MARIMBA_SLAVE_ID_CDC_ADDR	0x77
#define MARIMBA_SLAVE_ID_QMEMBIST_ADDR	0X66

#define BAHAMA_SLAVE_ID_FM_ADDR         0x2A
#define BAHAMA_SLAVE_ID_QMEMBIST_ADDR   0x7B

static const char *tsadc_id = "MADC";
static const char *vregs_tsadc_name[] = {
	"gp12",
	"s2",
};
static struct vreg *vregs_tsadc[ARRAY_SIZE(vregs_tsadc_name)];

static const char *vregs_timpani_tsadc_name[] = {
	"s3",
	"gp12",
	"gp16"
};
static struct vreg *vregs_timpani_tsadc[ARRAY_SIZE(vregs_timpani_tsadc_name)];

static int marimba_tsadc_power(int vreg_on)
{
	int i, rc = 0;
	int tsadc_adie_type = adie_get_detected_codec_type();

	if (tsadc_adie_type == TIMPANI_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_timpani_tsadc_name); i++) {
			if (!vregs_timpani_tsadc[i]) {
				pr_err("%s: vreg_get %s failed(%d)\n",
				__func__, vregs_timpani_tsadc_name[i], rc);
				goto vreg_fail;
			}

			rc = vreg_on ? vreg_enable(vregs_timpani_tsadc[i]) :
				  vreg_disable(vregs_timpani_tsadc[i]);
			if (rc < 0) {
				pr_err("%s: vreg %s %s failed(%d)\n",
					__func__, vregs_timpani_tsadc_name[i],
				       vreg_on ? "enable" : "disable", rc);
				goto vreg_fail;
			}
		}
		/* Vote for D0 and D1 buffer */
		rc = pmapp_clock_vote(tsadc_id, PMAPP_CLOCK_ID_D1,
			vreg_on ? PMAPP_CLOCK_VOTE_ON : PMAPP_CLOCK_VOTE_OFF);
		if (rc)	{
			pr_err("%s: unable to %svote for d1 clk\n",
				__func__, vreg_on ? "" : "de-");
			goto do_vote_fail;
		}
		rc = pmapp_clock_vote(tsadc_id, PMAPP_CLOCK_ID_DO,
			vreg_on ? PMAPP_CLOCK_VOTE_ON : PMAPP_CLOCK_VOTE_OFF);
		if (rc)	{
			pr_err("%s: unable to %svote for d1 clk\n",
				__func__, vreg_on ? "" : "de-");
			goto do_vote_fail;
		}
	} else if (tsadc_adie_type == MARIMBA_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_tsadc_name); i++) {
			if (!vregs_tsadc[i]) {
				pr_err("%s: vreg_get %s failed (%d)\n",
					__func__, vregs_tsadc_name[i], rc);
				goto vreg_fail;
			}

			rc = vreg_on ? vreg_enable(vregs_tsadc[i]) :
				  vreg_disable(vregs_tsadc[i]);
			if (rc < 0) {
				pr_err("%s: vreg %s %s failed (%d)\n",
					__func__, vregs_tsadc_name[i],
				       vreg_on ? "enable" : "disable", rc);
				goto vreg_fail;
			}
		}
		/* If marimba vote for DO buffer */
		rc = pmapp_clock_vote(tsadc_id, PMAPP_CLOCK_ID_DO,
			vreg_on ? PMAPP_CLOCK_VOTE_ON : PMAPP_CLOCK_VOTE_OFF);
		if (rc)	{
			pr_err("%s: unable to %svote for d0 clk\n",
				__func__, vreg_on ? "" : "de-");
			goto do_vote_fail;
		}
	} else {
		pr_err("%s:Adie %d not supported\n",
				__func__, tsadc_adie_type);
		return -ENODEV;
	}

	msleep(5); /* ensure power is stable */

	return 0;

do_vote_fail:
vreg_fail:
	while (i) {
		if (vreg_on) {
			if (tsadc_adie_type == TIMPANI_ID)
				vreg_disable(vregs_timpani_tsadc[--i]);
			else if (tsadc_adie_type == MARIMBA_ID)
				vreg_disable(vregs_tsadc[--i]);
		} else {
			if (tsadc_adie_type == TIMPANI_ID)
				vreg_enable(vregs_timpani_tsadc[--i]);
			else if (tsadc_adie_type == MARIMBA_ID)
				vreg_enable(vregs_tsadc[--i]);
		}
	}

	return rc;
}

static int marimba_tsadc_vote(int vote_on)
{
	int rc = 0;

	if (adie_get_detected_codec_type() == MARIMBA_ID) {
		int level = vote_on ? 1300 : 0;
		rc = pmapp_vreg_level_vote(tsadc_id, PMAPP_VREG_S2, level);
		if (rc < 0)
			pr_err("%s: vreg level %s failed (%d)\n",
			__func__, vote_on ? "on" : "off", rc);
	}

	return rc;
}

static int marimba_tsadc_init(void)
{
	int i, rc = 0;
	int tsadc_adie_type = adie_get_detected_codec_type();

	if (tsadc_adie_type == TIMPANI_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_timpani_tsadc_name); i++) {
			vregs_timpani_tsadc[i] = vreg_get(NULL,
						vregs_timpani_tsadc_name[i]);
			if (IS_ERR(vregs_timpani_tsadc[i])) {
				pr_err("%s: vreg get %s failed (%ld)\n",
				       __func__, vregs_timpani_tsadc_name[i],
				       PTR_ERR(vregs_timpani_tsadc[i]));
				rc = PTR_ERR(vregs_timpani_tsadc[i]);
				goto vreg_get_fail;
			}
		}
	} else if (tsadc_adie_type == MARIMBA_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_tsadc_name); i++) {
			vregs_tsadc[i] = vreg_get(NULL, vregs_tsadc_name[i]);
			if (IS_ERR(vregs_tsadc[i])) {
				pr_err("%s: vreg get %s failed (%ld)\n",
				       __func__, vregs_tsadc_name[i],
				       PTR_ERR(vregs_tsadc[i]));
				rc = PTR_ERR(vregs_tsadc[i]);
				goto vreg_get_fail;
			}
		}
	} else {
		pr_err("%s:Adie %d not supported\n",
				__func__, tsadc_adie_type);
		return -ENODEV;
	}

	return 0;

vreg_get_fail:
	while (i) {
		if (tsadc_adie_type == TIMPANI_ID)
			vreg_put(vregs_timpani_tsadc[--i]);
		else if (tsadc_adie_type == MARIMBA_ID)
			vreg_put(vregs_tsadc[--i]);
	}
	return rc;
}

static int marimba_tsadc_exit(void)
{
	int i, rc = 0;
	int tsadc_adie_type = adie_get_detected_codec_type();

	if (tsadc_adie_type == TIMPANI_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_timpani_tsadc_name); i++) {
			if (vregs_tsadc[i])
				vreg_put(vregs_timpani_tsadc[i]);
		}
	} else if (tsadc_adie_type == MARIMBA_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_tsadc_name); i++) {
			if (vregs_tsadc[i])
				vreg_put(vregs_tsadc[i]);
		}
		rc = pmapp_vreg_level_vote(tsadc_id, PMAPP_VREG_S2, 0);
		if (rc < 0)
			pr_err("%s: vreg level off failed (%d)\n",
						__func__, rc);
	} else {
		pr_err("%s:Adie %d not supported\n",
				__func__, tsadc_adie_type);
		rc = -ENODEV;
	}

	return rc;
}


static struct msm_ts_platform_data msm_ts_data = {
	.min_x          = 0,
	.max_x          = 4096,
	.min_y          = 0,
	.max_y          = 4096,
	.min_press      = 0,
	.max_press      = 255,
	.inv_x          = 4096,
	.inv_y          = 4096,
	.can_wakeup	= false,
};

static struct marimba_tsadc_platform_data marimba_tsadc_pdata = {
	.marimba_tsadc_power =  marimba_tsadc_power,
	.init		     =  marimba_tsadc_init,
	.exit		     =  marimba_tsadc_exit,
	.level_vote	     =  marimba_tsadc_vote,
	.tsadc_prechg_en = true,
	.can_wakeup	= false,
	.setup = {
		.pen_irq_en	=	true,
		.tsadc_en	=	true,
	},
	.params2 = {
		.input_clk_khz		=	2400,
		.sample_prd		=	TSADC_CLK_3,
	},
	.params3 = {
		.prechg_time_nsecs	=	6400,
		.stable_time_nsecs	=	6400,
		.tsadc_test_mode	=	0,
	},
	.tssc_data = &msm_ts_data,
};

static struct vreg *vreg_codec_s4;
static int msm_marimba_codec_power(int vreg_on)
{
	int rc = 0;

	if (!vreg_codec_s4) {

		vreg_codec_s4 = vreg_get(NULL, "s4");

		if (IS_ERR(vreg_codec_s4)) {
			printk(KERN_ERR "%s: vreg_get() failed (%ld)\n",
				__func__, PTR_ERR(vreg_codec_s4));
			rc = PTR_ERR(vreg_codec_s4);
			goto  vreg_codec_s4_fail;
		}
	}

	if (vreg_on) {
		rc = vreg_enable(vreg_codec_s4);
		if (rc)
			printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		goto vreg_codec_s4_fail;
	} else {
		rc = vreg_disable(vreg_codec_s4);
		if (rc)
			printk(KERN_ERR "%s: vreg_disable() = %d \n",
					__func__, rc);
		goto vreg_codec_s4_fail;
	}

vreg_codec_s4_fail:
	return rc;
}

static struct marimba_codec_platform_data mariba_codec_pdata = {
	.marimba_codec_power =  msm_marimba_codec_power,
#ifdef CONFIG_MARIMBA_CODEC
	.snddev_profile_init = msm_snddev_init,
#endif
};

static struct marimba_platform_data marimba_pdata = {
	.slave_id[MARIMBA_SLAVE_ID_FM]       = MARIMBA_SLAVE_ID_FM_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_CDC]	     = MARIMBA_SLAVE_ID_CDC_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_QMEMBIST] = MARIMBA_SLAVE_ID_QMEMBIST_ADDR,
	.slave_id[SLAVE_ID_BAHAMA_FM]        = BAHAMA_SLAVE_ID_FM_ADDR,
	.slave_id[SLAVE_ID_BAHAMA_QMEMBIST]  = BAHAMA_SLAVE_ID_QMEMBIST_ADDR,
	.marimba_setup = msm_marimba_setup_power,
	.marimba_shutdown = msm_marimba_shutdown_power,
	.bahama_setup = msm_bahama_setup_power,
	.bahama_shutdown = msm_bahama_shutdown_power,
	.marimba_gpio_config = msm_marimba_gpio_config_svlte,
	.bahama_core_config = msm_bahama_core_config,
	.fm = &marimba_fm_pdata,
	.codec = &mariba_codec_pdata,
};

static void __init msm7x30_init_marimba(void)
{
	int rc;

	vreg_marimba_1 = vreg_get(NULL, "s3");
	if (IS_ERR(vreg_marimba_1)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_marimba_1));
		return;
	}
	rc = vreg_set_level(vreg_marimba_1, 1800);

	vreg_marimba_2 = vreg_get(NULL, "gp16");
	if (IS_ERR(vreg_marimba_1)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_marimba_1));
		return;
	}
	rc = vreg_set_level(vreg_marimba_2, 1200);

	vreg_marimba_3 = vreg_get(NULL, "usb2");
	if (IS_ERR(vreg_marimba_3)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_marimba_3));
		return;
	}
	rc = vreg_set_level(vreg_marimba_3, 1800);
}

#ifdef CONFIG_TIMPANI_CODEC
static struct marimba_codec_platform_data timpani_codec_pdata = {
	.marimba_codec_power =  msm_marimba_codec_power,
#ifdef CONFIG_TIMPANI_CODEC
	.snddev_profile_init = msm_snddev_init_timpani,
#endif
};

static struct marimba_platform_data timpani_pdata = {
	.slave_id[MARIMBA_SLAVE_ID_CDC]	= MARIMBA_SLAVE_ID_CDC_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_QMEMBIST] = MARIMBA_SLAVE_ID_QMEMBIST_ADDR,
	.marimba_setup = msm_timpani_setup_power,
	.marimba_shutdown = msm_timpani_shutdown_power,
	.codec = &timpani_codec_pdata,
	.tsadc = &marimba_tsadc_pdata,
};

#define TIMPANI_I2C_SLAVE_ADDR	0xD

static struct i2c_board_info msm_i2c_gsbi7_timpani_info[] = {
	{
		I2C_BOARD_INFO("timpani", TIMPANI_I2C_SLAVE_ADDR),
		.platform_data = &timpani_pdata,
	},
};
#endif

#ifdef CONFIG_MSM7KV2_AUDIO
static struct resource msm_aictl_resources[] = {
	{
		.name = "aictl",
		.start = 0xa5000100,
		.end = 0xa5000100,
		.flags = IORESOURCE_MEM,
	}
};

static struct resource msm_mi2s_resources[] = {
	{
		.name = "hdmi",
		.start = 0xac900000,
		.end = 0xac900038,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "codec_rx",
		.start = 0xac940040,
		.end = 0xac940078,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "codec_tx",
		.start = 0xac980080,
		.end = 0xac9800B8,
		.flags = IORESOURCE_MEM,
	}

};

static struct msm_lpa_platform_data lpa_pdata = {
	.obuf_hlb_size = 0x2BFF8,
	.dsp_proc_id = 0,
	.app_proc_id = 2,
	.nosb_config = {
		.llb_min_addr = 0,
		.llb_max_addr = 0x3ff8,
		.sb_min_addr = 0,
		.sb_max_addr = 0,
	},
	.sb_config = {
		.llb_min_addr = 0,
		.llb_max_addr = 0x37f8,
		.sb_min_addr = 0x3800,
		.sb_max_addr = 0x3ff8,
	}
};

static struct resource msm_lpa_resources[] = {
	{
		.name = "lpa",
		.start = 0xa5000000,
		.end = 0xa50000a0,
		.flags = IORESOURCE_MEM,
	}
};

static struct resource msm_aux_pcm_resources[] = {

	{
		.name = "aux_codec_reg_addr",
		.start = 0xac9c00c0,
		.end = 0xac9c00c8,
		.flags = IORESOURCE_MEM,
	},
	{
		.name   = "aux_pcm_dout",
		.start  = 138,
		.end    = 138,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_din",
		.start  = 139,
		.end    = 139,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_syncout",
		.start  = 140,
		.end    = 140,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_clkin_a",
		.start  = 141,
		.end    = 141,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device msm_aux_pcm_device = {
	.name   = "msm_aux_pcm",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_aux_pcm_resources),
	.resource       = msm_aux_pcm_resources,
};

struct platform_device msm_aictl_device = {
	.name = "audio_interct",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_aictl_resources),
	.resource = msm_aictl_resources,
};

struct platform_device msm_mi2s_device = {
	.name = "mi2s",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_mi2s_resources),
	.resource = msm_mi2s_resources,
};

struct platform_device msm_lpa_device = {
	.name = "lpa",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_lpa_resources),
	.resource = msm_lpa_resources,
	.dev		= {
		.platform_data = &lpa_pdata,
	},
};
#endif /* CONFIG_MSM7KV2_AUDIO */

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
 #define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
 #define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	0,
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_MODE_LP)|
	(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 1 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),

	 /* Concurrency 2 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 3 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 4 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 5 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 6 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

#define DEC_INSTANCE(max_instance_same, max_instance_diff) { \
	.max_instances_same_dec = max_instance_same, \
	.max_instances_diff_dec = max_instance_diff}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
};

static struct dec_instance_table dec_instance_list[][MSM_MAX_DEC_CNT] = {
	/* Non Turbo Mode */
	{
		DEC_INSTANCE(4, 3), /* WAV */
		DEC_INSTANCE(4, 3), /* ADPCM */
		DEC_INSTANCE(4, 2), /* MP3 */
		DEC_INSTANCE(0, 0), /* Real Audio */
		DEC_INSTANCE(4, 2), /* WMA */
		DEC_INSTANCE(3, 2), /* AAC */
		DEC_INSTANCE(0, 0), /* Reserved */
		DEC_INSTANCE(0, 0), /* MIDI */
		DEC_INSTANCE(4, 3), /* YADPCM */
		DEC_INSTANCE(4, 3), /* QCELP */
		DEC_INSTANCE(4, 3), /* AMRNB */
		DEC_INSTANCE(1, 1), /* AMRWB/WB+ */
		DEC_INSTANCE(4, 3), /* EVRC */
		DEC_INSTANCE(1, 1), /* WMAPRO */
	},
	/* Turbo Mode */
	{
		DEC_INSTANCE(4, 3), /* WAV */
		DEC_INSTANCE(4, 3), /* ADPCM */
		DEC_INSTANCE(4, 3), /* MP3 */
		DEC_INSTANCE(0, 0), /* Real Audio */
		DEC_INSTANCE(4, 3), /* WMA */
		DEC_INSTANCE(4, 3), /* AAC */
		DEC_INSTANCE(0, 0), /* Reserved */
		DEC_INSTANCE(0, 0), /* MIDI */
		DEC_INSTANCE(4, 3), /* YADPCM */
		DEC_INSTANCE(4, 3), /* QCELP */
		DEC_INSTANCE(4, 3), /* AMRNB */
		DEC_INSTANCE(2, 3), /* AMRWB/WB+ */
		DEC_INSTANCE(4, 3), /* EVRC */
		DEC_INSTANCE(1, 2), /* WMAPRO */
	},
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
	.dec_instance_list = &dec_instance_list[0][0],
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

static struct resource smc91x_resources[] = {
	[0] = {
		.start = 0x8A000300,
		.end = 0x8A0003ff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start = MSM_GPIO_TO_INT(156),
		.end = MSM_GPIO_TO_INT(156),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device smc91x_device = {
	.name           = "smc91x",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(smc91x_resources),
	.resource       = smc91x_resources,
};

static struct smsc911x_platform_config smsc911x_config = {
	.phy_interface	= PHY_INTERFACE_MODE_MII,
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type	= SMSC911X_IRQ_TYPE_PUSH_PULL,
	.flags		= SMSC911X_USE_32BIT,
};

static struct resource smsc911x_resources[] = {
	[0] = {
		.start		= 0x8D000000,
		.end		= 0x8D000100,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= MSM_GPIO_TO_INT(88),
		.end		= MSM_GPIO_TO_INT(88),
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device smsc911x_device = {
	.name		= "smsc911x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smsc911x_resources),
	.resource	= smsc911x_resources,
	.dev		= {
		.platform_data = &smsc911x_config,
	},
};

static struct msm_gpio smsc911x_gpios[] = {
    { GPIO_CFG(172, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr6" },
    { GPIO_CFG(173, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr5" },
    { GPIO_CFG(174, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr4" },
    { GPIO_CFG(175, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr3" },
    { GPIO_CFG(176, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr2" },
    { GPIO_CFG(177, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr1" },
    { GPIO_CFG(178, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "ebi2_addr0" },
    { GPIO_CFG(88, 2, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "smsc911x_irq"  },
};

static void msm7x30_cfg_smsc911x(void)
{
	int rc;

	rc = msm_gpios_request_enable(smsc911x_gpios,
			ARRAY_SIZE(smsc911x_gpios));
	if (rc)
		pr_err("%s: unable to enable gpios\n", __func__);
}

// BEGIN: 0009214 sehyuny.kim@lge.com 2010-09-03
// MOD 0009214: [DIAG] LG Diag feature added in side of android
static struct diagcmd_platform_data lg_fw_diagcmd_pdata = {	
	.name = "lg_fw_diagcmd",
};

static struct platform_device lg_fw_diagcmd_device = {	
	.name = "lg_fw_diagcmd",	
	.id = -1,	
	.dev    = 	{		
		.platform_data = &lg_fw_diagcmd_pdata	
				},
};

static struct platform_device lg_diag_cmd_device = {	
	.name = "lg_diag_cmd",	
	.id = -1,	
	.dev    = 	{		
		.platform_data = 0, //&lg_diag_cmd_pdata	
				},
};
// END: 0009214 sehyuny.kim@lge.com 2010-09-03


/* BEGIN:0011986 [yk.kim@lge.com] 2010-12-07 */
/* ADD:0011986 Bryce USB composition redefine */
#ifdef CONFIG_USB_ANDROID
#ifdef CONFIG_LGE_USB_GADGET_DRIVER
#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
#ifdef CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER
static char *usb_functions_all[] = {
	"acm",
	"diag",
	"cdc_ethernet",
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_AUTORUN
	"usb_autorun",
#endif
#ifdef CONFIG_LGE_USB_GADGET_MTP_DRIVER
	"mtp",
#endif
};
#endif
#endif

#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
#ifdef CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER
static char *usb_functions_ums[] = {
	"usb_mass_storage",
};
#endif
#endif

#ifdef CONFIG_LGE_USB_GADGET_SUPPORT_FACTORY_USB
static char *usb_functions_factory[] = {
	"acm",
	"diag",
};
#endif /* CONFIG_LGE_USB_GADGET_SUPPORT_FACTORY_USB */

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_AUTORUN 
static char *usb_functions_autorun[] = {
	"usb_autorun",
};
#endif

#ifdef CONFIG_LGE_USB_GADGET_MTP_DRIVER
static char *usb_functions_mtp[] = {
	"mtp",
};
#endif

#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
#ifdef CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER
static char *usb_functions_ndis[] = {
	"acm",
	"diag",
	"cdc_ethernet",
	"adb",
};

#endif
#endif /* CONFIG_LGE_USB_GADGET_NDIS_DRIVER */

static struct android_usb_product usb_products[] = {
#ifdef CONFIG_LGE_USB_GADGET_SUPPORT_FACTORY_USB
	{
		.product_id	= 0x6000,
		.num_functions	= ARRAY_SIZE(usb_functions_factory),
		.functions	= usb_functions_factory,
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
		.unique_function = FACTORY,
#endif
	},
#endif /* CONFIG_LGE_USB_GADGET_SUPPORT_FACTORY_USB */
#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
#ifdef CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER
	{
		.product_id = 0x6200,
		.num_functions	= ARRAY_SIZE(usb_functions_ndis),
		.functions	= usb_functions_ndis,
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
		.unique_function = NDIS,
#endif	
	},
	{
		.product_id = 0x6308,
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
		.unique_function = UMS,
#endif	
	},

#endif /* CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER */
#endif /* CONFIG_LGE_USB_GADGET_NDIS_DRIVER */
#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_AUTORUN
	{
		.product_id	= 0x6203,
		.num_functions	= ARRAY_SIZE(usb_functions_autorun),
		.functions	= usb_functions_autorun,
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
		.unique_function = CD_ROM,
#endif	
	},
#endif /* CONFIG_USB_SUPPORT_LGE_ANDROID_AUTORUN */
#ifdef CONFIG_LGE_USB_GADGET_MTP_DRIVER
	{
		.product_id = 0x6202,
		.num_functions	= ARRAY_SIZE(usb_functions_mtp),
		.functions	= usb_functions_mtp,
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
		.unique_function = MTP,
#endif	
	},
#endif /* CONFIG_LGE_USB_GADGET_MTP_DRIVER */
};

#ifdef CONFIG_LGE_USB_GADGET_SUPPORT_FACTORY_USB
struct android_usb_platform_data android_usb_pdata_factory = {
	.vendor_id	= 0x1004,
	.product_id	= 0x6000,
	.version	= 0x0100,
	.product_name		= "LGE CDMA Composite USB Device",
	.manufacturer_name	= "LG Electronics Inc.",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_factory),
	.functions = usb_functions_factory,
	.serial_number = "\0",
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
	.unique_function = FACTORY,
#endif	
};
#endif /* CONFIG_LGE_USB_GADGET_SUPPORT_FACTORY_USB */
#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
#ifdef CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER
struct android_usb_platform_data android_usb_pdata_ndis = {
	.vendor_id	= 0x1004,
	.product_id	= 0x6200,
	.version	= 0x0100,
	.product_name		= "LG Android USB Device",
	.manufacturer_name	= "LG Electronics Inc.",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.serial_number = "LGANDROIDVS910",
#ifdef CONFIG_LGE_USB_GADGET_FUNC_BIND_ONLY_INIT
	.unique_function = NDIS,
#endif
};
#endif /* CONFIG_LGE_USB_GADGET_NDIS_VZW_DRIVER */
#endif /* CONFIG_LGE_USB_GADGET_NDIS_DRIVER */
#endif /* CONFIG_LGE_USB_GADGET_DRIVER */

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
	.platform_data = &android_usb_pdata_ndis,
#endif
	},
};

#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
static struct usb_ether_platform_data ecm_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x1004,
	.vendorDescr	= "LG Electronics Inc.",
};

static struct platform_device ecm_device = {
	.name	= "cdc_ethernet",
	.id	= -1,
	.dev	= {
		.platform_data = &ecm_pdata,
	},
};
#endif

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 2,
	.vendor		= "LG Electronics Inc.",
	.product        = "Mass storage",
	.release	= 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

#endif /* CONFIG_USB_ANDROID */
/* END:0011986 [yk.kim@lge.com] 2010-12-07 */


static struct msm_gpio optnav_config_data[] = {
	{ GPIO_CFG(OPTNAV_CHIP_SELECT, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	"optnav_chip_select" },
};

static void __iomem *virtual_optnav;

static int optnav_gpio_setup(void)
{
	int rc = -ENODEV;
	rc = msm_gpios_request_enable(optnav_config_data,
			ARRAY_SIZE(optnav_config_data));

	/* Configure the FPGA for GPIOs */
	virtual_optnav = ioremap(FPGA_OPTNAV_GPIO_ADDR, 0x4);
	if (!virtual_optnav) {
		pr_err("%s:Could not ioremap region\n", __func__);
		return -ENOMEM;
	}
	/*
	 * Configure the FPGA to set GPIO 19 as
	 * normal, active(enabled), output(MSM to SURF)
	 */
	writew(0x311E, virtual_optnav);
	return rc;
}

static void optnav_gpio_release(void)
{
	msm_gpios_disable_free(optnav_config_data,
		ARRAY_SIZE(optnav_config_data));
	iounmap(virtual_optnav);
}

static struct vreg *vreg_gp7;
static struct vreg *vreg_gp4;
static struct vreg *vreg_gp9;
static struct vreg *vreg_usb3_3;

static int optnav_enable(void)
{
	int rc;
	/*
	 * Enable the VREGs L8(gp7), L10(gp4), L12(gp9), L6(usb)
	 * for I2C communication with keyboard.
	 */
	vreg_gp7 = vreg_get(NULL, "gp7");
	rc = vreg_set_level(vreg_gp7, 1800);
	if (rc) {
		pr_err("%s: vreg_set_level failed \n", __func__);
		goto fail_vreg_gp7;
	}

	rc = vreg_enable(vreg_gp7);
	if (rc) {
		pr_err("%s: vreg_enable failed \n", __func__);
		goto fail_vreg_gp7;
	}

	vreg_gp4 = vreg_get(NULL, "gp4");
	rc = vreg_set_level(vreg_gp4, 2600);
	if (rc) {
		pr_err("%s: vreg_set_level failed \n", __func__);
		goto fail_vreg_gp4;
	}

	rc = vreg_enable(vreg_gp4);
	if (rc) {
		pr_err("%s: vreg_enable failed \n", __func__);
		goto fail_vreg_gp4;
	}

	vreg_gp9 = vreg_get(NULL, "gp9");
	rc = vreg_set_level(vreg_gp9, 1800);
	if (rc) {
		pr_err("%s: vreg_set_level failed \n", __func__);
		goto fail_vreg_gp9;
	}

	rc = vreg_enable(vreg_gp9);
	if (rc) {
		pr_err("%s: vreg_enable failed \n", __func__);
		goto fail_vreg_gp9;
	}

	vreg_usb3_3 = vreg_get(NULL, "usb");
	rc = vreg_set_level(vreg_usb3_3, 3300);
	if (rc) {
		pr_err("%s: vreg_set_level failed \n", __func__);
		goto fail_vreg_3_3;
	}

	rc = vreg_enable(vreg_usb3_3);
	if (rc) {
		pr_err("%s: vreg_enable failed \n", __func__);
		goto fail_vreg_3_3;
	}

	/* Enable the chip select GPIO */
	gpio_set_value(OPTNAV_CHIP_SELECT, 1);
	gpio_set_value(OPTNAV_CHIP_SELECT, 0);

	return 0;

fail_vreg_3_3:
	vreg_disable(vreg_gp9);
fail_vreg_gp9:
	vreg_disable(vreg_gp4);
fail_vreg_gp4:
	vreg_disable(vreg_gp7);
fail_vreg_gp7:
	return rc;
}

static void optnav_disable(void)
{
	vreg_disable(vreg_usb3_3);
	vreg_disable(vreg_gp9);
	vreg_disable(vreg_gp4);
	vreg_disable(vreg_gp7);

	gpio_set_value(OPTNAV_CHIP_SELECT, 1);
}

static struct ofn_atlab_platform_data optnav_data = {
	.gpio_setup    = optnav_gpio_setup,
	.gpio_release  = optnav_gpio_release,
	.optnav_on     = optnav_enable,
	.optnav_off    = optnav_disable,
	.rotate_xy     = 0,
	.function1 = {
		.no_motion1_en		= true,
		.touch_sensor_en	= true,
		.ofn_en			= true,
		.clock_select_khz	= 1500,
		.cpi_selection		= 1200,
	},
	.function2 =  {
		.invert_y		= false,
		.invert_x		= true,
		.swap_x_y		= false,
		.hold_a_b_en		= true,
		.motion_filter_en       = true,
	},
};

static struct msm_hdmi_platform_data adv7520_hdmi_data = {
		.irq = MSM_GPIO_TO_INT(18),
};

/*  jaeseong.gim	10.07.19
	hdmi	start
*/

#if defined(LG_HW_REV1) || defined(LG_HW_REV2)
#define GPIO_I2C_HDMI_SDA	82
#define GPIO_I2C_HDMI_SCL	83
/* kenneth.kang 2010-11-17 [Start] [modify] modify HW revision info to build */
#else // defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6)
#define GPIO_I2C_HDMI_SDA	82
#define GPIO_I2C_HDMI_SCL	90
#endif
/* kenneth.kang 2010-11-17 [END] */
#define TX_NAME "tda998X"
#define CEC_NAME "tda998Xcec"
#define TDA998X_I2C_SLAVEADDRESS 0x70
#define TDA99XCEC_I2C_SLAVEADDRESS 0x34

static struct gpio_i2c_pin hdmi_i2c_pin[] = {
	[0] = {
        .sda_pin = GPIO_I2C_HDMI_SDA,
        .scl_pin = GPIO_I2C_HDMI_SCL,
	},
};

static struct i2c_gpio_platform_data hdmi_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device hdmi_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &hdmi_i2c_pdata,
};


static struct i2c_board_info __initdata hdmi_i2c_bdinfo[] = {
	{I2C_BOARD_INFO(TX_NAME,  TDA998X_I2C_SLAVEADDRESS),},
	{I2C_BOARD_INFO(CEC_NAME,  TDA99XCEC_I2C_SLAVEADDRESS),},
};


void __init bryce_init_i2c_hdmi(int bus_num)
{
	printk("%s, bus_num=%d\n", __func__, bus_num);
	hdmi_i2c_device.id = bus_num;
	init_gpio_i2c_pin(&hdmi_i2c_pdata, hdmi_i2c_pin[0], &hdmi_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, hdmi_i2c_bdinfo, ARRAY_SIZE(hdmi_i2c_bdinfo));
	platform_device_register(&hdmi_i2c_device);
}
/*  jaeseong.gim	10.07.19
	hdmi	 end
*/
/* sungmin.shin	10.07.10
	backlight device & touch
*/
#if CONFIG_MACH_LGE_BRYCE
static struct backlight_platform_data bd6091gu_data = {
	.gpio = 96, //LCD_CP_RESET_N
	.max_current = 0x58, /* 17.8 mA */
	.init_on_boot = 1,
};

static int mddi_lgit_pmic_bl(int level)
{
	return 0;
}

static struct msm_panel_common_pdata mddi_lgit_pdata = {
	.pmic_backlight = mddi_lgit_pmic_bl,
};

static struct platform_device mddi_lgit_device = {
	.name   = "mddi_lgit",
	.id     = 0,
	.dev    = {
		.platform_data = &mddi_lgit_pdata,
	}
};

struct spi_gpio_platform_data amri5k_spi_platform_data = {
	.sck = 45,
	.mosi = 47,
	.miso = 48,
	.num_chipselect = 1,
};

static struct platform_device amri5k_spi_device = {
	.name		= "spi_gpio",
	.dev	= {
		.platform_data	= &amri5k_spi_platform_data,
	},
};

struct amri5k_platform_data {
	int touchmode;	
	//int (*setup)(struct device *);
	//void (*teardown)(struct device *);
};

static struct amri5k_platform_data amri5k_pdata = {
	.touchmode = 0x04,
};

static struct spi_board_info msm_spi_board_info[] __initdata = {
	{
		.modalias	= "AMRI5K",
		.mode		= SPI_MODE_3,
		.irq		= MSM_GPIO_TO_INT(107),
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 2000000,
		.platform_data	= &amri5k_pdata,
		.controller_data = 46,
	}
};
#endif 
#ifdef CONFIG_BOSCH_BMA150
static struct vreg *vreg_gp6;
static int sensors_ldo_enable(void)
{
	int rc;

	/*
	 * Enable the VREGs L8(gp7), L15(gp6)
	 * for I2C communication with sensors.
	 */
	pr_info("sensors_ldo_enable called!!\n");
	vreg_gp7 = vreg_get(NULL, "gp7");
	if (IS_ERR(vreg_gp7)) {
		pr_err("%s: vreg_get gp7 failed\n", __func__);
		rc = PTR_ERR(vreg_gp7);
		goto fail_gp7_get;
	}

	rc = vreg_set_level(vreg_gp7, 1800);
	if (rc) {
		pr_err("%s: vreg_set_level gp7 failed\n", __func__);
		goto fail_gp7_level;
	}

	rc = vreg_enable(vreg_gp7);
	if (rc) {
		pr_err("%s: vreg_enable gp7 failed\n", __func__);
		goto fail_gp7_level;
	}

	vreg_gp6 = vreg_get(NULL, "gp6");
	if (IS_ERR(vreg_gp6)) {
		pr_err("%s: vreg_get gp6 failed\n", __func__);
		rc = PTR_ERR(vreg_gp6);
		goto fail_gp6_get;
	}

	rc = vreg_set_level(vreg_gp6, 2800);
	if (rc) {
		pr_err("%s: vreg_set_level gp6 failed\n", __func__);
		goto fail_gp6_level;
	}

	rc = vreg_enable(vreg_gp6);
	if (rc) {
		pr_err("%s: vreg_enable gp6 failed\n", __func__);
		goto fail_gp6_level;
	}

	return 0;

fail_gp6_level:
	vreg_put(vreg_gp6);
fail_gp6_get:
	vreg_disable(vreg_gp7);
fail_gp7_level:
	vreg_put(vreg_gp7);
fail_gp7_get:
	return rc;
}

static void sensors_ldo_disable(void)
{
	pr_info("sensors_ldo_disable called!!\n");
	vreg_disable(vreg_gp6);
	vreg_put(vreg_gp6);
	vreg_disable(vreg_gp7);
	vreg_put(vreg_gp7);
}
static struct bma150_platform_data bma150_data = {
	.power_on = sensors_ldo_enable,
	.power_off = sensors_ldo_disable,
};

static struct i2c_board_info bma150_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("bma150", 0x38),
		.flags = I2C_CLIENT_WAKE,
		.irq = MSM_GPIO_TO_INT(BMA150_GPIO_INT),
		.platform_data = &bma150_data,
	},
};
#endif
static struct i2c_board_info msm_i2c_board_info[] = {
/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	backlight device
*/
#if CONFIG_MACH_LGE_BRYCE
#if defined(LG_HW_REV1)
	[0] = {
		I2C_BOARD_INFO("bd6091gu", 0xEC >> 1),
		.platform_data = &bd6091gu_data,
	},
#endif 	
/* 2010.07.29 ey.cho 
	touch in RevA
	TODO. add touch platform data later
*/
	[0] = {
		I2C_BOARD_INFO("t1310", 0x20),
		
	/*	.platform_data = &touch_data,*/
	},
#endif 
};

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	backlight device
*/
#if CONFIG_MACH_LGE_BRYCE
#ifdef LG_HW_REV2
static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("bd6091gu", 0xEC >> 1),
		.platform_data = &bd6091gu_data,
	},
};

static struct gpio_i2c_pin bl_i2c_pin[] = {
	[0] = {
		.sda_pin	= 1,
		.scl_pin	= 0,
	},
};

static struct i2c_gpio_platform_data bl_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device bl_i2c_device = {
	.id = 12,
	.name = "i2c-gpio",
	.dev.platform_data = &bl_i2c_pdata,
};
#endif

/* jihye.ahn	10.07.29
	backlight for Rev.A
*/
/* jihye.ahn@lge.com   10-09-20   add Rev.C board feature */
#if defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
static struct backlight_platform_data lm3528_data = {
	.gpio = 25, //LCD_HW_EN
	.max_current = 0x7F, /* 17.8 mA */
	.init_on_boot = 1,
};
//In Rev C TPA Amp Shares this I2C. ++ kiran.kanneganti@lge.com baikal 0009680
#if defined (LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("lm3528", 0x6C >> 1),
		.platform_data = &lm3528_data,
	},
	[1] = {
		I2C_BOARD_INFO("tpa2055", 0xE0>> 1),
	},
};
#else
static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("lm3528", 0x6C >> 1),
		.platform_data = &lm3528_data,
	},
};
#endif
// -- kiran.kanneganti@lge.com baikal 0009680
static struct gpio_i2c_pin bl_i2c_pin[] = {
	[0] = {
		.sda_pin	= 1,
		.scl_pin	= 0,
	},
};

static struct i2c_gpio_platform_data bl_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device bl_i2c_device = {
	.id = 12,
	.name = "i2c-gpio",
	.dev.platform_data = &bl_i2c_pdata,
};
#endif
#endif 


/* kwangdo.yi 		010810
   apds power set function added for sensors.
   */
#ifdef CONFIG_SENSOR_APDS9900

// BEGIN 0011662: eundeok.bae@lge.com APDS9900 LDO MODIFIED
// [LDO] Fixed a problem that APDS9900 had a wrong power source.
#if 0
#define VREG_PROXI_VDD_26V "xo_out"
#define VREG_PROXI_LED_26V "rf"
#endif
#define VREG_PROXI_LED_26V "xo_out"
#define APDS_IRQ 50
// END 0011662: eundeok.bae@lge.com APDS9900 LDO MODIFIED

static int apds_power_set(unsigned char enable)
{
  int err = 0;
  struct vreg *vreg_power_1;
  struct vreg *vreg_power_2;
  struct vreg *vreg_power_3;

  printk("### apds_power_set enable : %d ", enable);
// BEGIN 0011662: eundeok.bae@lge.com APDS9900 LDO MODIFIED
// [LDO] Fixed a problem that APDS9900 had a wrong power source.
#if 0
	vreg_power_1 = vreg_get(0, VREG_PROXI_VDD_26V);
	vreg_power_2 = vreg_get(0, VREG_PROXI_LED_26V);
#endif
  vreg_power_1 = vreg_get(0, VREG_PROXI_LED_26V);
  vreg_power_2 = vreg_get(0, VREG_PERI_26V);
  vreg_power_3 = vreg_get(0, VREG_SENSOR_IO_18V);
// END 0011662: eundeok.bae@lge.com APDS9900 LDO MODIFIED

  if (enable) {
    vreg_enable(vreg_power_1);
    err = vreg_set_level(vreg_power_1, 2600);
    if (err != 0) {
      printk("### vreg_power_1 failed.\n");
      return -1;
    }

    vreg_enable(vreg_power_2);
    err = vreg_set_level(vreg_power_2, 2600);
    if (err != 0) {
      printk("### vreg_power_2 failed.\n");
      return -1;
    }

    vreg_enable(vreg_power_3);
    err = vreg_set_level(vreg_power_3, 1800);
    if (err != 0) {
      printk("### vreg_power_2 failed.\n");
      return -1;
    }

				
    printk("### adps sensor power OK\n");
  }
  else {
    vreg_disable(vreg_power_1);
    vreg_disable(vreg_power_2);
  }

  return err; 
}
#endif
/* sungmin.shin	10.07.28
	ALS device
*/
#ifdef CONFIG_SENSOR_APDS9900
static struct apds9900_platform_data apds9900_pdata = {
#if defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	.power = apds_power_set,
	.irq_num= APDS_IRQ,
#endif
};
#endif

/* kwangdo.yi 		010706
   added for sensors.
 */
#ifdef CONFIG_MACH_LGE_BRYCE
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
/* acceleration */
static int kr3dh_config_gpio(unsigned char config)
{
	int err = 0;

	/*GPIO setting*/
        err = gpio_request(GPIO_ACCEL_INT, "kr3dh");
	if (err != 0)
		printk("GPIO_TOUCH_ATTN request failed.\n");

	gpio_direction_input(GPIO_ACCEL_INT);
	err = gpio_tlmm_config(GPIO_CFG(GPIO_ACCEL_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	return err;
}

static int kr_init(void){return 0;}
static void kr_exit(void){}
static int power_on(void)
{
	int err = 0;
	struct vreg *vreg_power_1;
	struct vreg *vreg_power_2;
	vreg_power_1= vreg_get(0, VREG_PERI_26V);
	vreg_power_2 = vreg_get(0, VREG_SENSOR_IO_18V);
	vreg_enable(vreg_power_1);
	err = vreg_set_level(vreg_power_1, 2600);
	if (err != 0) 
	{
		printk("###kr3dh  vreg_power_1 failed.\n");
		return -1;
	}

	vreg_enable(vreg_power_2);
	err = vreg_set_level(vreg_power_2, 1800);
	if (err != 0) 
	{
		printk("### kr3dhvreg_power_2 failed.\n");
		return -1;
	}
	printk("### kr3dhsensor power OK\n");
	return 0;
}
static int power_off(void)
{
	struct vreg *vreg_power_1;
	struct vreg *vreg_power_2;
	vreg_power_1 = vreg_get(0, VREG_PERI_26V);
	vreg_power_2 = vreg_get(0, VREG_SENSOR_IO_18V);
	vreg_disable(vreg_power_1);
	vreg_disable(vreg_power_2);
	printk("### kr3dhsensor power off\n");
	return 0;
}

struct kr3dh_platform_data accel_pdata = {
	.poll_interval = 100,
	.min_interval = 0,
	.g_range = 0x00,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,

	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 0,

	.power_on = power_on,
	.power_off = power_off,
	.kr_init = kr_init,
	.kr_exit = kr_exit,
	.gpio_config = kr3dh_config_gpio,
};

/* ecompass */
static int ecom_power_set(unsigned char enable)
{
	int err = 0;
	struct vreg *vreg_power_1;
	struct vreg *vreg_power_2;

	printk("### ecom_power_set enable : %d ", enable);

	vreg_power_1 = vreg_get(0, VREG_PERI_26V);
        vreg_power_2 = vreg_get(0, VREG_SENSOR_IO_18V);

	if (enable) {
		vreg_enable(vreg_power_1);
		err = vreg_set_level(vreg_power_1, 2600);
		if (err != 0) {
			printk("### vreg_power_1 failed.\n");
			return -1;
		}

		vreg_enable(vreg_power_2);
		err = vreg_set_level(vreg_power_2, 1800);
		if (err != 0) {
			printk("### vreg_power_2 failed.\n");
			return -1;
		}
		printk("### sensor power OK\n");
	}
	else {
		vreg_disable(vreg_power_1);
		vreg_disable(vreg_power_2);
	}

	return err;
}

static s16 m_hlayout[2][9] ={
	{-1, 0, 0, 0, -1, 0, 0, 0, 1},
	{0, 1, 0, -1, 0, 0, 0, 0, 1}
};

static s16 m_alayout[2][9] = {
	{1, 0, 0, 0, 1, 0, 0, 0, 1},
	{0, -1, 0, 1, 0, 0, 0, 0, 1}
};

static struct ecom_platform_data ecom_pdata = {
	.pin_int        = GPIO_COMPASS_IRQ,
	.pin_rst        = GPIO_COMPASS_RST,
	.power          = ecom_power_set,
	.accelerator_name = "kr3dh",
	.fdata_sign_x = -1,
	.fdata_sign_y = -1,
	.fdata_sign_z = -1,
	.fdata_order0 = 1,
	.fdata_order1 = 0,
	.fdata_order2 = 2,
	.sensitivity1g = 1024,
	.h_layout = m_hlayout,
	.a_layout = m_alayout,
};

static struct gpio_i2c_pin sensor_i2c_pin[] = {
	[0] = {
		.sda_pin        = GPIO_I2C_SENSOR_SDA,
		.scl_pin        = GPIO_I2C_SENSOR_SCL,
		.reset_pin      = GPIO_COMPASS_RST,
		.irq_pin        = GPIO_COMPASS_IRQ,
	},
};

static struct i2c_gpio_platform_data sensor_i2c_pdata = {
	.sda_pin	= GPIO_I2C_SENSOR_SDA,
	.scl_pin	= GPIO_I2C_SENSOR_SCL,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device sensor_i2c_device = {
	.id = 15,	
	.name = "i2c-gpio",
	.dev.platform_data = &sensor_i2c_pdata,
};

static struct i2c_board_info sensor_i2c_info[] = {
	{
		I2C_BOARD_INFO("akm8973", 0x1c),
		.platform_data = &ecom_pdata,
	},

	{
		/* kwangdo.yi 		100706
			fix I2C address of motion sensor 0x18->0x19
		*/
#ifdef CONFIG_MACH_LGE_BRYCE
#if defined (LG_HW_REV2) || defined (LG_HW_REV3) || defined (LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)

		I2C_BOARD_INFO("kr3dh", 0x19),
#else
		I2C_BOARD_INFO("kr3dh", 0x18),
#endif
#endif
		.platform_data = &accel_pdata,
	},
/* sungmin.shin	10.07.30
	add proxi and ambient light sensor
*/
#ifdef CONFIG_SENSOR_APDS9900
#if defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	{
		I2C_BOARD_INFO("apds9900", 0x39),
		.platform_data = &apds9900_pdata,
	},
#endif
#endif 
};
#endif
#endif

static struct i2c_board_info msm_marimba_board_info[] = {
	{
		I2C_BOARD_INFO("marimba", 0xc),
		.platform_data = &marimba_pdata,
	}
};

static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency = 8594,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].residency = 23740,

	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency = 4594,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].residency = 23740,

	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].supported = 1,
#ifdef CONFIG_MSM_STANDALONE_POWER_COLLAPSE
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].suspend_enabled = 0,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].idle_enabled = 1,
#else /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].suspend_enabled = 0,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].idle_enabled = 0,
#endif /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].latency = 500,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].residency = 6000,

	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].supported = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].suspend_enabled
		= 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].idle_enabled = 0,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency = 443,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].residency = 1098,

	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].supported = 1,
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].latency = 2,
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].residency = 0,
};

static struct resource qsd_spi_resources[] = {
	{
		.name   = "spi_irq_in",
		.start	= INT_SPI_INPUT,
		.end	= INT_SPI_INPUT,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_out",
		.start	= INT_SPI_OUTPUT,
		.end	= INT_SPI_OUTPUT,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_err",
		.start	= INT_SPI_ERROR,
		.end	= INT_SPI_ERROR,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name   = "spi_base",
		.start	= 0xA8000000,
		.end	= 0xA8000000 + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name   = "spidm_channels",
		.flags  = IORESOURCE_DMA,
	},
	{
		.name   = "spidm_crci",
		.flags  = IORESOURCE_DMA,
	},
};

#define AMDH0_BASE_PHYS		0xAC200000
#define ADMH0_GP_CTL		(ct_adm_base + 0x3D8)
static int msm_qsd_spi_dma_config(void)
{
	void __iomem *ct_adm_base = 0;
	u32 spi_mux = 0;
	int ret = 0;

	ct_adm_base = ioremap(AMDH0_BASE_PHYS, PAGE_SIZE);
	if (!ct_adm_base) {
		pr_err("%s: Could not remap %x\n", __func__, AMDH0_BASE_PHYS);
		return -ENOMEM;
	}

	spi_mux = (ioread32(ADMH0_GP_CTL) & (0x3 << 12)) >> 12;

	qsd_spi_resources[4].start  = DMOV_USB_CHAN;
	qsd_spi_resources[4].end    = DMOV_TSIF_CHAN;

	switch (spi_mux) {
	case (1):
		qsd_spi_resources[5].start  = DMOV_HSUART1_RX_CRCI;
		qsd_spi_resources[5].end    = DMOV_HSUART1_TX_CRCI;
		break;
	case (2):
		qsd_spi_resources[5].start  = DMOV_HSUART2_RX_CRCI;
		qsd_spi_resources[5].end    = DMOV_HSUART2_TX_CRCI;
		break;
	case (3):
		qsd_spi_resources[5].start  = DMOV_CE_OUT_CRCI;
		qsd_spi_resources[5].end    = DMOV_CE_IN_CRCI;
		break;
	default:
		ret = -ENOENT;
	}

	iounmap(ct_adm_base);

	return ret;
}

static struct platform_device qsd_device_spi = {
	.name		= "spi_qsd",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qsd_spi_resources),
	.resource	= qsd_spi_resources,
};

#ifdef CONFIG_SPI_QSD
static struct spi_board_info lcdc_sharp_spi_board_info[] __initdata = {
	{
		.modalias	= "lcdc_sharp_ls038y7dx01",
		.mode		= SPI_MODE_1,
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 26331429,
	}
};
static struct spi_board_info lcdc_toshiba_spi_board_info[] __initdata = {
	{
		.modalias       = "lcdc_toshiba_ltm030dd40",
		.mode           = SPI_MODE_3|SPI_CS_HIGH,
		.bus_num        = 0,
		.chip_select    = 0,
		.max_speed_hz   = 9963243,
	}
};
#endif

static struct msm_gpio qsd_spi_gpio_config_data[] = {
	{ GPIO_CFG(45, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_clk" },
	{ GPIO_CFG(46, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_cs0" },
	{ GPIO_CFG(47, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "spi_mosi" },
	{ GPIO_CFG(48, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_miso" },
};

static int msm_qsd_spi_gpio_config(void)
{
	return msm_gpios_request_enable(qsd_spi_gpio_config_data,
		ARRAY_SIZE(qsd_spi_gpio_config_data));
}

static void msm_qsd_spi_gpio_release(void)
{
	msm_gpios_disable_free(qsd_spi_gpio_config_data,
		ARRAY_SIZE(qsd_spi_gpio_config_data));
}

static struct msm_spi_platform_data qsd_spi_pdata = {
	.max_clock_speed = 26331429,
	.clk_name = "spi_clk",
	.pclk_name = "spi_pclk",
	.gpio_config  = msm_qsd_spi_gpio_config,
	.gpio_release = msm_qsd_spi_gpio_release,
	.dma_config = msm_qsd_spi_dma_config,
};

static void __init msm_qsd_spi_init(void)
{
	qsd_device_spi.dev.platform_data = &qsd_spi_pdata;
}

#ifdef CONFIG_USB_EHCI_MSM
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	int rc;
	static int vbus_is_on;
	struct pm8058_gpio usb_vbus = {
		.direction      = PM_GPIO_DIR_OUT,
		.pull           = PM_GPIO_PULL_NO,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 1,
		.vin_sel        = 2,
		.out_strength   = PM_GPIO_STRENGTH_MED,
		.function       = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol    = 0,
	};

	/* If VBUS is already on (or off), do nothing. */
	if (unlikely(on == vbus_is_on))
		return;

	if (on) {
		rc = pm8058_gpio_config(36, &usb_vbus);
		if (rc) {
			pr_err("%s PMIC GPIO 36 write failed\n", __func__);
			return;
		}
	} else {
		gpio_set_value_cansleep(PM8058_GPIO_PM_TO_SYS(36), 0);
	}

	vbus_is_on = on;
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info   = (USB_PHY_INTEGRATED | USB_PHY_MODEL_45NM),
	.vbus_power = msm_hsusb_vbus_power,
	.power_budget   = 180,
};
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
static int hsusb_rpc_connect(int connect)
{
	if (connect)
		return msm_hsusb_rpc_connect();
	else
		return msm_hsusb_rpc_close();
}
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
static struct vreg *vreg_3p3;
static int msm_hsusb_ldo_init(int init)
{
	uint32_t version = 0;
	int def_vol = 3400;

	version = socinfo_get_version();

	if (SOCINFO_VERSION_MAJOR(version) >= 2 &&
			SOCINFO_VERSION_MINOR(version) >= 1) {
		def_vol = 3075;
		pr_debug("%s: default voltage:%d\n", __func__, def_vol);
	}

	if (init) {
		vreg_3p3 = vreg_get(NULL, "usb");
		if (IS_ERR(vreg_3p3))
			return PTR_ERR(vreg_3p3);
		vreg_set_level(vreg_3p3, def_vol);
	} else
		vreg_put(vreg_3p3);

	return 0;
}

static int msm_hsusb_ldo_enable(int enable)
{
	static int ldo_status;

	if (!vreg_3p3 || IS_ERR(vreg_3p3))
		return -ENODEV;

	if (ldo_status == enable)
		return 0;

	ldo_status = enable;

	if (enable)
		return vreg_enable(vreg_3p3);

	return vreg_disable(vreg_3p3);
}

static int msm_hsusb_ldo_set_voltage(int mV)
{
	static int cur_voltage = 3400;

	if (!vreg_3p3 || IS_ERR(vreg_3p3))
		return -ENODEV;

	if (cur_voltage == mV)
		return 0;

	cur_voltage = mV;

	pr_debug("%s: (%d)\n", __func__, mV);

	return vreg_set_level(vreg_3p3, mV);
}
#endif
#ifndef CONFIG_USB_EHCI_MSM
static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init);
#endif
static struct msm_otg_platform_data msm_otg_pdata = {
	.rpc_connect	= hsusb_rpc_connect,

#ifndef CONFIG_USB_EHCI_MSM
	.pmic_vbus_notif_init         = msm_hsusb_pmic_notif_init,
#else
	.vbus_power = msm_hsusb_vbus_power,
#endif
	.core_clk		 = 1,
	.pemp_level		 = PRE_EMPHASIS_WITH_20_PERCENT,
	.cdr_autoreset		 = CDR_AUTO_RESET_DISABLE,
	.drv_ampl		 = HS_DRV_AMPLITUDE_DEFAULT,
	.se1_gating		 = SE1_GATING_DISABLE,
	.chg_vbus_draw		 = hsusb_chg_vbus_draw,
	.chg_connected		 = hsusb_chg_connected,
	.chg_init		 = hsusb_chg_init,
	.ldo_enable		 = msm_hsusb_ldo_enable,
	.ldo_init		 = msm_hsusb_ldo_init,
	.ldo_set_voltage	 = msm_hsusb_ldo_set_voltage,
};

#ifdef CONFIG_USB_GADGET
#if 0 //Kiran.kanneganti. wall charger not working with timer.
//According to Froyo
static struct msm_hsusb_gadget_platform_data msm_gadget_pdata = {
	.is_phy_status_timer_on = 1,
};
#else
static struct msm_hsusb_gadget_platform_data msm_gadget_pdata;
#endif
#endif

#ifndef CONFIG_USB_EHCI_MSM
typedef void (*notify_vbus_state) (int);
notify_vbus_state notify_vbus_state_func_ptr;
int vbus_on_irq;
static irqreturn_t pmic_vbus_on_irq(int irq, void *data)
{
	pr_info("%s: vbus notification from pmic\n", __func__);

	(*notify_vbus_state_func_ptr) (1);

	return IRQ_HANDLED;
}
static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init)
{
	int ret;

	if (init) {
		if (!callback)
			return -ENODEV;

		notify_vbus_state_func_ptr = callback;
		vbus_on_irq = platform_get_irq_byname(&msm_device_otg,
			"vbus_on");
		if (vbus_on_irq <= 0) {
			pr_err("%s: unable to get vbus on irq\n", __func__);
			return -ENODEV;
		}

		ret = request_any_context_irq(vbus_on_irq, pmic_vbus_on_irq,
			IRQF_TRIGGER_RISING, "msm_otg_vbus_on", NULL);
		if (ret < 0) {
			pr_info("%s: request_irq for vbus_on"
				"interrupt failed\n", __func__);
			return ret;
		}
		msm_otg_pdata.pmic_vbus_irq = vbus_on_irq;
		return 0;
	} else {
		free_irq(vbus_on_irq, 0);
		notify_vbus_state_func_ptr = NULL;
		return 0;
	}
}
#endif

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

#ifndef CONFIG_SPI_QSD
static int lcdc_gpio_array_num[] = {
				45, /* spi_clk */
				46, /* spi_cs  */
				47, /* spi_mosi */
				48, /* spi_miso */
				};

static struct msm_gpio lcdc_gpio_config_data[] = {
	{ GPIO_CFG(45, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_clk" },
	{ GPIO_CFG(46, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_cs0" },
	{ GPIO_CFG(47, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_mosi" },
	{ GPIO_CFG(48, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_miso" },
};

static void lcdc_config_gpios(int enable)
{
	if (enable) {
		msm_gpios_request_enable(lcdc_gpio_config_data,
					      ARRAY_SIZE(
						      lcdc_gpio_config_data));
	} else
		msm_gpios_disable_free(lcdc_gpio_config_data,
					    ARRAY_SIZE(
						    lcdc_gpio_config_data));
}
#endif

static struct msm_panel_common_pdata lcdc_sharp_panel_data = {
#ifndef CONFIG_SPI_QSD
	.panel_config_gpio = lcdc_config_gpios,
	.gpio_num          = lcdc_gpio_array_num,
#endif
	.gpio = 2, 	/* LPG PMIC_GPIO26 channel number */
};

static struct platform_device lcdc_sharp_panel_device = {
	.name   = "lcdc_sharp_wvga",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_sharp_panel_data,
	}
};

static struct msm_gpio dtv_panel_gpios[] = {
/* jaeseong.gim 	2010.07.12
                for hdmi 
 */
#ifndef CONFIG_MACH_LGE_BRYCE
	{ GPIO_CFG(18, 0, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "hdmi_int" },
	{ GPIO_CFG(120, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "wca_mclk" },
	{ GPIO_CFG(121, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "wca_sd0" },
	{ GPIO_CFG(122, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "wca_sd1" },
	{ GPIO_CFG(123, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "wca_sd2" },
#endif
	{ GPIO_CFG(124, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "dtv_pclk" },
	{ GPIO_CFG(125, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_en" },
	{ GPIO_CFG(126, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_vsync" },
	{ GPIO_CFG(127, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_hsync" },
	{ GPIO_CFG(128, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data0" },
	{ GPIO_CFG(129, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data1" },
	{ GPIO_CFG(130, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data2" },
	{ GPIO_CFG(131, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data3" },
	{ GPIO_CFG(132, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data4" },
	{ GPIO_CFG(160, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data5" },
	{ GPIO_CFG(161, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data6" },
	{ GPIO_CFG(162, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data7" },
	{ GPIO_CFG(163, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data8" },
	{ GPIO_CFG(164, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_data9" },
	{ GPIO_CFG(165, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat10" },
	{ GPIO_CFG(166, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat11" },
	{ GPIO_CFG(167, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat12" },
	{ GPIO_CFG(168, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat13" },
	{ GPIO_CFG(169, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat14" },
	{ GPIO_CFG(170, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat15" },
	{ GPIO_CFG(171, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat16" },
	{ GPIO_CFG(172, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat17" },
	{ GPIO_CFG(173, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat18" },
	{ GPIO_CFG(174, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat19" },
	{ GPIO_CFG(175, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat20" },
	{ GPIO_CFG(176, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat21" },
	{ GPIO_CFG(177, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat22" },
	{ GPIO_CFG(178, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_4MA), "dtv_dat23" },
};


/* jaeseong.gim 	2010.07.12
                for hdmi 
 */
#ifdef CONFIG_MACH_LGE_BRYCE
#define HDMI_RESET
#endif
#ifdef HDMI_RESET
static unsigned dtv_reset_gpio =
	GPIO_CFG(109, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA);
#endif

static int gpio_set(const char *label, const char *name, int level, int on)
{
	struct vreg *vreg = vreg_get(NULL, label);
	int rc;

	if (IS_ERR(vreg)) {
		rc = PTR_ERR(vreg);
		pr_err("%s: vreg %s get failed (%d)\n",
			__func__, name, rc);
		return rc;
	}

	rc = vreg_set_level(vreg, level);
	if (rc) {
		pr_err("%s: vreg %s set level failed (%d)\n",
			__func__, name, rc);
		return rc;
	}

	if (on)
		rc = vreg_enable(vreg);
	else
		rc = vreg_disable(vreg);
	if (rc)
		pr_err("%s: vreg %s enable failed (%d)\n",
			__func__, name, rc);
	return rc;
}

static int i2c_gpio_power(int on)
{
	int rc = gpio_set("gp7", "LDO8", 1800, on);
	if (rc)
		return rc;
	return gpio_set("gp4", "LDO10", 2600, on);
}
static int dtv_power_save_on=0;
static void dtv_panel_power(int on)
/* jaeseong.gim 	2010.07.12
                for hdmi 
 */
#ifdef CONFIG_MACH_LGE_BRYCE
{
	int flag_on = !!on;
	int rc;

	if (dtv_power_save_on == flag_on)
		return;
	printk("[jaeseong.gim]%s, power %s\n",__func__,on?"on":"off");
	dtv_power_save_on = flag_on;
	if (on) {
		rc = gpio_tlmm_config(dtv_reset_gpio, GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, dtv_reset_gpio, rc);
			return;
		}
		gpio_set_value(109, 1);	/* bring reset line high */
		mdelay(10);		/* 10 msec before IO can be accessed */
	}
	else{
		//gpio_set_value(109, 0);	/* bring reset line high */
	}
}
#else

{
	int flag_on = !!on;
	static int dtv_power_save_on;
	struct vreg *vreg_ldo17, *vreg_ldo8;
	int rc;

	if (dtv_power_save_on == flag_on)
		return 0;

	dtv_power_save_on = flag_on;
	pr_info("%s: %d >>\n", __func__, on);

	gpio_set_value_cansleep(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_HDMI_5V_EN),
		on);

#ifdef HDMI_RESET
	if (on) {
		/* reset Toshiba WeGA chip -- toggle reset pin -- gpio_180 */
		rc = gpio_tlmm_config(dtv_reset_gpio, GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, dtv_reset_gpio, rc);
			return rc;
		}

		gpio_set_value(37, 0);	/* bring reset line low to hold reset*/
	}
#endif

	if (on) {
		rc = msm_gpios_enable(dtv_panel_gpios,
				ARRAY_SIZE(dtv_panel_gpios));
		if (rc < 0) {
			printk(KERN_ERR "%s: gpio enable failed: %d\n",
				__func__, rc);
			return rc;
		}
	} else {
		rc = msm_gpios_disable(dtv_panel_gpios,
				ARRAY_SIZE(dtv_panel_gpios));
		if (rc < 0) {
			printk(KERN_ERR "%s: gpio disable failed: %d\n",
				__func__, rc);
			return rc;
		}
	}

	rc = i2c_gpio_power(on);
	if (rc)
		return rc;

	mdelay(5);		/* ensure power is stable */

	/*  -- LDO17 for HDMI */
	rc = gpio_set("gp11", "LDO17", 2600, on);
	if (rc)
		return rc;

	mdelay(5);		/* ensure power is stable */

#ifdef HDMI_RESET
	if (on) {
		gpio_set_value(37, 1);	/* bring reset line high */
		mdelay(10);		/* 10 msec before IO can be accessed */
	}
#endif
	pr_info("%s: %d <<\n", __func__, on);

	return rc;
}

static struct lcdc_platform_data dtv_pdata = {
	.lcdc_power_save   = dtv_panel_power,
};
#endif

/* jaeseong.gim 	2010.07.12
                for hdmi 
 */
#ifdef CONFIG_MACH_LGE_BRYCE
static void dtv_gpio_config(int enable){
	if(enable)
		msm_gpios_enable(dtv_panel_gpios,ARRAY_SIZE(dtv_panel_gpios));
	else
		msm_gpios_disable(dtv_panel_gpios,ARRAY_SIZE(dtv_panel_gpios));
}

static struct lcdc_platform_data dtv_pdata = {
	.lcdc_power_save   = dtv_panel_power,
	.lcdc_gpio_config  = dtv_gpio_config,
};
#endif

static struct platform_device hdmi_adv7520_panel_device = {
	.name   = "adv7520",
	.id     = 0,
};

static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
       .inject_rx_on_wakeup = 1,
       .rx_to_inject = 0xFD,
};

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
	if (!strncmp(name, "mddi_toshiba_wvga_pt", 20))
		return -EPERM;
	else if (!strncmp(name, "lcdc_toshiba_wvga_pt", 20))
		return 0;
	else if (!strcmp(name, "mddi_orise"))
		return -EPERM;
	else
		return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
	.mddi_prescan = 1,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

static struct platform_device msm_migrate_pages_device = {
	.name   = "msm_migrate_pages",
	.id     = -1,
};

static struct android_pmem_platform_data android_pmem_kernel_ebi1_pdata = {
       .name = PMEM_KERNEL_EBI1_DATA_NAME,
	/* if no allocator_type, defaults to PMEM_ALLOCATORTYPE_BITMAP,
	* the only valid choice at this time. The board structure is
	* set to all zeros by the C runtime initialization and that is now
	* the enum value of PMEM_ALLOCATORTYPE_BITMAP, now forced to 0 in
	* include/linux/android_pmem.h.
	*/
       .cached = 0,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
       .name = "pmem_adsp",
       .allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
       .cached = 0,
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
       .name = "pmem_audio",
       .allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
       .cached = 0,
};

static struct platform_device android_pmem_kernel_ebi1_device = {
       .name = "android_pmem",
       .id = 1,
       .dev = { .platform_data = &android_pmem_kernel_ebi1_pdata },
};

static struct platform_device android_pmem_adsp_device = {
       .name = "android_pmem",
       .id = 2,
       .dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct platform_device android_pmem_audio_device = {
       .name = "android_pmem",
       .id = 4,
       .dev = { .platform_data = &android_pmem_audio_pdata },
};

static struct kgsl_platform_data kgsl_pdata = {
#ifdef CONFIG_MSM_NPA_SYSTEM_BUS
	/* NPA Flow IDs */
	.high_axi_3d = MSM_AXI_FLOW_3D_GPU_HIGH,
	.high_axi_2d = MSM_AXI_FLOW_2D_GPU_HIGH,
#else
	/* AXI rates in KHz */
	.high_axi_3d = 192000,
	.high_axi_2d = 192000,
#endif
	.max_grp2d_freq = 0,
	.min_grp2d_freq = 0,
	.set_grp2d_async = NULL, /* HW workaround, run Z180 SYNC @ 192 MHZ */
	.max_grp3d_freq = 245760000,
	.min_grp3d_freq = 192 * 1000*1000,
	.set_grp3d_async = set_grp3d_async,
	.imem_clk_name = "imem_clk",
	.grp3d_clk_name = "grp_clk",
	.grp3d_pclk_name = "grp_pclk",
#ifdef CONFIG_MSM_KGSL_2D
	.grp2d0_clk_name = "grp_2d_clk",
	.grp2d0_pclk_name = "grp_2d_pclk",
#else
	.grp2d0_clk_name = NULL,
#endif
	.idle_timeout_3d = HZ/20,
	.idle_timeout_2d = HZ/10,
	.nap_allowed = true,

#ifdef CONFIG_KGSL_PER_PROCESS_PAGE_TABLE
	.pt_va_size = SZ_32M,
	/* Maximum of 32 concurrent processes */
	.pt_max_count = 32,
#else
	.pt_va_size = SZ_128M,
	/* We only ever have one pagetable for everybody */
	.pt_max_count = 1,
#endif
};

static struct resource kgsl_resources[] = {
	{
		.name = "kgsl_reg_memory",
		.start = 0xA3500000, /* 3D GRP address */
		.end = 0xA351ffff,
		.flags = IORESOURCE_MEM,
	},
/*	{
		.name   = "kgsl_phys_memory",
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_MEM,
	},*/ //lavanya commented
	{
		.name = "kgsl_yamato_irq",
		.start = INT_GRP_3D,
		.end = INT_GRP_3D,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name = "kgsl_2d0_reg_memory",
		.start = 0xA3900000, /* Z180 base address */
		.end = 0xA3900FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "kgsl_2d0_irq",
		.start = INT_GRP_2D,
		.end = INT_GRP_2D,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device msm_device_kgsl = {
	.name = "kgsl",
	.id = -1,
	.num_resources = ARRAY_SIZE(kgsl_resources),
	.resource = kgsl_resources,
	.dev = {
		.platform_data = &kgsl_pdata,
	},
};

static int mddi_toshiba_pmic_bl(int level)
{
	int ret = -EPERM;

	ret = pmic_set_led_intensity(LED_LCD, level);

	if (ret)
		printk(KERN_WARNING "%s: can't set lcd backlight!\n",
					__func__);
	return ret;
}

static struct msm_panel_common_pdata mddi_toshiba_pdata = {
	.pmic_backlight = mddi_toshiba_pmic_bl,
};

static struct platform_device mddi_toshiba_device = {
	.name   = "mddi_toshiba",
	.id     = 0,
	.dev    = {
		.platform_data = &mddi_toshiba_pdata,
	}
};

static unsigned wega_reset_gpio =
	GPIO_CFG(180, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA);

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

static int display_common_power(int on)
{
	int rc = 0;  
	/* not used, related with fluid board */
	#if 0
	static int display_common_power_save_on;

	int flag_on = !!on;
	struct vreg *vreg_ldo12, *vreg_ldo15 = NULL;
	struct vreg *vreg_ldo20, *vreg_ldo16, *vreg_ldo8 = NULL; 
	#endif 

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.16
	comment out
*/
#if CONFIG_MACH_LGE_BRYCE

#else
	if (display_common_power_save_on == flag_on)
		return 0;

	display_common_power_save_on = flag_on;

	if (on) {
		/* reset Toshiba WeGA chip -- toggle reset pin -- gpio_180 */
		rc = gpio_tlmm_config(wega_reset_gpio, GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, wega_reset_gpio, rc);
			return rc;
		}

		gpio_set_value(180, 0);	/* bring reset line low to hold reset*/
	}

	/* Toshiba WeGA power -- has 3 power source */
	/* 1.5V -- LDO20*/
	vreg_ldo20 = vreg_get(NULL, "gp13");

	if (IS_ERR(vreg_ldo20)) {
		rc = PTR_ERR(vreg_ldo20);
		pr_err("%s: gp13 vreg get failed (%d)\n",
		       __func__, rc);
		return rc;
	}

	/* 1.8V -- LDO12 */
	vreg_ldo12 = vreg_get(NULL, "gp9");

	if (IS_ERR(vreg_ldo12)) {
		rc = PTR_ERR(vreg_ldo12);
		pr_err("%s: gp9 vreg get failed (%d)\n",
		       __func__, rc);
		return rc;
	}

	/* 2.6V -- LDO16 */
	vreg_ldo16 = vreg_get(NULL, "gp10");

	if (IS_ERR(vreg_ldo16)) {
		rc = PTR_ERR(vreg_ldo16);
		pr_err("%s: gp10 vreg get failed (%d)\n",
		       __func__, rc);
		return rc;
	}

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

	/* lcd panel power */
	/* 3.1V -- LDO15 */
	vreg_ldo15 = vreg_get(NULL, "gp6");

	if (IS_ERR(vreg_ldo15)) {
		rc = PTR_ERR(vreg_ldo15);
		pr_err("%s: gp6 vreg get failed (%d)\n",
			__func__, rc);
		return rc;
	}

	rc = vreg_set_level(vreg_ldo20, 1500);
	if (rc) {
		pr_err("%s: vreg LDO20 set level failed (%d)\n",
		       __func__, rc);
		return rc;
	}

	rc = vreg_set_level(vreg_ldo12, 1800);
	if (rc) {
		pr_err("%s: vreg LDO12 set level failed (%d)\n",
		       __func__, rc);
		return rc;
	}

	rc = vreg_set_level(vreg_ldo16, 2600);
	if (rc) {
		pr_err("%s: vreg LDO16 set level failed (%d)\n",
		       __func__, rc);
		return rc;
	}

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
	rc = vreg_set_level(vreg_ldo15, 3100);
	if (rc) {
		pr_err("%s: vreg LDO15 set level failed (%d)\n",
			__func__, rc);
		return rc;
	}

	if (on) {
		rc = vreg_enable(vreg_ldo20);
		if (rc) {
			pr_err("%s: LDO20 vreg enable failed (%d)\n",
			       __func__, rc);
			return rc;
		}

		rc = vreg_enable(vreg_ldo12);
		if (rc) {
			pr_err("%s: LDO12 vreg enable failed (%d)\n",
			       __func__, rc);
			return rc;
		}

		rc = vreg_enable(vreg_ldo16);
		if (rc) {
			pr_err("%s: LDO16 vreg enable failed (%d)\n",
			       __func__, rc);
			return rc;
		}

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
		rc = vreg_enable(vreg_ldo15);
		if (rc) {
			pr_err("%s: LDO15 vreg enable failed (%d)\n",
				__func__, rc);
			return rc;
		}

		mdelay(5);		/* ensure power is stable */

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

		gpio_set_value(180, 1);	/* bring reset line high */
		mdelay(10);	/* 10 msec before IO can be accessed */
		rc = pmapp_display_clock_config(1);
		if (rc) {
			pr_err("%s pmapp_display_clock_config rc=%d\n",
					__func__, rc);
			return rc;
		}

	} else {
		rc = vreg_disable(vreg_ldo20);
		if (rc) {
			pr_err("%s: LDO20 vreg enable failed (%d)\n",
			       __func__, rc);
			return rc;
		}


		rc = vreg_disable(vreg_ldo16);
		if (rc) {
			pr_err("%s: LDO16 vreg enable failed (%d)\n",
			       __func__, rc);
			return rc;
		}

		gpio_set_value(180, 0);	/* bring reset line low */

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
		rc = vreg_disable(vreg_ldo15);
		if (rc) {
			pr_err("%s: LDO15 vreg enable failed (%d)\n",
				__func__, rc);
			return rc;
		}

		mdelay(5);	/* ensure power is stable */

		rc = vreg_disable(vreg_ldo12);
		if (rc) {
			pr_err("%s: LDO12 vreg enable failed (%d)\n",
			       __func__, rc);
			return rc;
		}

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

		rc = pmapp_display_clock_config(0);
		if (rc) {
			pr_err("%s pmapp_display_clock_config rc=%d\n",
					__func__, rc);
			return rc;
		}
	}
#endif 

	return rc;
}

static int msm_fb_mddi_sel_clk(u32 *clk_rate)
{
	*clk_rate *= 2;
	return 0;
}

static struct mddi_platform_data mddi_pdata = {
	.mddi_power_save = display_common_power,
	.mddi_sel_clk = msm_fb_mddi_sel_clk,
};

int mdp_core_clk_rate_table[] = {
	122880000,
	122880000,
	122880000,
	192000000,
};

static struct msm_panel_common_pdata mdp_pdata = {
/*Video play & pause --> Lock Screen --> unlock --> blue screen.*/
   /*Keep Hw revision address in GB. kiran.kanneganti@lge.com*/
	.hw_revision_addr = 0xac001270,
	.gpio = 30,
	.mdp_core_clk_rate = 122880000,
	.mdp_core_clk_table = mdp_core_clk_rate_table,
	.num_mdp_clk = ARRAY_SIZE(mdp_core_clk_rate_table),
};

static int lcd_panel_spi_gpio_num[] = {
			45, /* spi_clk */
			46, /* spi_cs  */
			47, /* spi_mosi */
			48, /* spi_miso */
		};

static struct msm_gpio lcd_panel_gpios[] = {
/* Workaround, since HDMI_INT is using the same GPIO line (18), and is used as
 * input.  if there is a hardware revision; we should reassign this GPIO to a
 * new open line; and removing it will just ensure that this will be missed in
 * the future.
	{ GPIO_CFG(18, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn0" },
 */
	{ GPIO_CFG(19, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn1" },
	{ GPIO_CFG(20, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu0" },
	{ GPIO_CFG(21, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu1" },
	{ GPIO_CFG(22, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu2" },
	{ GPIO_CFG(23, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red0" },
	{ GPIO_CFG(24, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red1" },
	{ GPIO_CFG(25, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red2" },
#ifndef CONFIG_SPI_QSD
	{ GPIO_CFG(45, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_clk" },
	{ GPIO_CFG(46, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_cs0" },
	{ GPIO_CFG(47, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_mosi" },
	{ GPIO_CFG(48, 0, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "spi_miso" },
#endif
	{ GPIO_CFG(90, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_pclk" },
	{ GPIO_CFG(91, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_en" },
	{ GPIO_CFG(92, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_vsync" },
	{ GPIO_CFG(93, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_hsync" },
	{ GPIO_CFG(94, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn2" },
	{ GPIO_CFG(95, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn3" },
	{ GPIO_CFG(96, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn4" },
	{ GPIO_CFG(97, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn5" },
	{ GPIO_CFG(98, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn6" },
	{ GPIO_CFG(99, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn7" },
	{ GPIO_CFG(100, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu3" },
	{ GPIO_CFG(101, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu4" },
	{ GPIO_CFG(102, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu5" },
	{ GPIO_CFG(103, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu6" },
	{ GPIO_CFG(104, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu7" },
	{ GPIO_CFG(105, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red3" },
	{ GPIO_CFG(106, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red4" },
	{ GPIO_CFG(107, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red5" },
	{ GPIO_CFG(108, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red6" },
	{ GPIO_CFG(109, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red7" },
};

static struct msm_gpio lcd_sharp_panel_gpios[] = {
	{ GPIO_CFG(22, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu2" },
	{ GPIO_CFG(25, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red2" },
	{ GPIO_CFG(90, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_pclk" },
	{ GPIO_CFG(91, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_en" },
	{ GPIO_CFG(92, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_vsync" },
	{ GPIO_CFG(93, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_hsync" },
	{ GPIO_CFG(94, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn2" },
	{ GPIO_CFG(95, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn3" },
	{ GPIO_CFG(96, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn4" },
	{ GPIO_CFG(97, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn5" },
	{ GPIO_CFG(98, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn6" },
	{ GPIO_CFG(99, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_grn7" },
	{ GPIO_CFG(100, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu3" },
	{ GPIO_CFG(101, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu4" },
	{ GPIO_CFG(102, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu5" },
	{ GPIO_CFG(103, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu6" },
	{ GPIO_CFG(104, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_blu7" },
	{ GPIO_CFG(105, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red3" },
	{ GPIO_CFG(106, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red4" },
	{ GPIO_CFG(107, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red5" },
	{ GPIO_CFG(108, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red6" },
	{ GPIO_CFG(109, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "lcdc_red7" },
};

static int lcdc_toshiba_panel_power(int on)
{
	int rc, i;
	struct msm_gpio *gp;

	rc = display_common_power(on);
	if (rc < 0) {
		printk(KERN_ERR "%s display_common_power failed: %d\n",
				__func__, rc);
		return rc;
	}

	if (on) {
		rc = msm_gpios_enable(lcd_panel_gpios,
				ARRAY_SIZE(lcd_panel_gpios));
		if (rc < 0) {
			printk(KERN_ERR "%s: gpio enable failed: %d\n",
					__func__, rc);
		}
	} else {	/* off */
		gp = lcd_panel_gpios;
		for (i = 0; i < ARRAY_SIZE(lcd_panel_gpios); i++) {
			/* ouput low */
			gpio_set_value(GPIO_PIN(gp->gpio_cfg), 0);
			gp++;
		}
	}

	return rc;
}

static int lcdc_sharp_panel_power(int on)
{
	int rc, i;
	struct msm_gpio *gp;

	rc = display_common_power(on);
	if (rc < 0) {
		printk(KERN_ERR "%s display_common_power failed: %d\n",
				__func__, rc);
		return rc;
	}

	if (on) {
		rc = msm_gpios_enable(lcd_sharp_panel_gpios,
				ARRAY_SIZE(lcd_sharp_panel_gpios));
		if (rc < 0) {
			printk(KERN_ERR "%s: gpio enable failed: %d\n",
				__func__, rc);
		}
	} else {	/* off */
		gp = lcd_sharp_panel_gpios;
		for (i = 0; i < ARRAY_SIZE(lcd_sharp_panel_gpios); i++) {
			/* ouput low */
			gpio_set_value(GPIO_PIN(gp->gpio_cfg), 0);
			gp++;
		}
	}

	return rc;
}

static int lcdc_panel_power(int on)
{
	int flag_on = !!on;
	static int lcdc_power_save_on;

	if (lcdc_power_save_on == flag_on)
		return 0;

	lcdc_power_save_on = flag_on;

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
	return lcdc_toshiba_panel_power(on);
}

static struct lcdc_platform_data lcdc_pdata = {
	.lcdc_power_save   = lcdc_panel_power,
};

static int atv_dac_power(int on)
{
	int rc = 0;
	struct vreg *vreg_s4, *vreg_ldo9;

	vreg_s4 = vreg_get(NULL, "s4");
	if (IS_ERR(vreg_s4)) {
		rc = PTR_ERR(vreg_s4);
		pr_err("%s: s4 vreg get failed (%d)\n",
			__func__, rc);
		return -1;
	}
	vreg_ldo9 = vreg_get(NULL, "gp1");
	if (IS_ERR(vreg_ldo9)) {
		rc = PTR_ERR(vreg_ldo9);
		pr_err("%s: ldo9 vreg get failed (%d)\n",
			__func__, rc);
		return rc;
	}

	if (on) {
		rc = vreg_enable(vreg_s4);
		if (rc) {
			pr_err("%s: s4 vreg enable failed (%d)\n",
				__func__, rc);
			return rc;
		}
		rc = vreg_enable(vreg_ldo9);
		if (rc) {
			pr_err("%s: ldo9 vreg enable failed (%d)\n",
				__func__, rc);
			return rc;
		}
	} else {
		rc = vreg_disable(vreg_ldo9);
		if (rc) {
			pr_err("%s: ldo9 vreg disable failed (%d)\n",
				   __func__, rc);
			return rc;
		}
		rc = vreg_disable(vreg_s4);
		if (rc) {
			pr_err("%s: s4 vreg disable failed (%d)\n",
				   __func__, rc);
			return rc;
		}
	}
	return rc;
}


static struct tvenc_platform_data atv_pdata = {
	.poll		 = 1,
	.pm_vid_en   = atv_dac_power,
};

/* neo.kang@lge.com	10.12.15. S
 * 0012867 : add the hidden reset 
 * To get the buffer for frame buffer for pass the osbl. 
 * The osbl maybe display this frame buffer after hidden reset 
 * So we expect that an end user doesn't know the phone crash. */
#if defined(CONFIG_LGE_HIDDEN_RESET_PATCH)
static size_t fb_copy_phys;
static size_t fb_copy_size;
static size_t fb_copy_virt;

void *lge_get_fb_addr(void)
{
	return (void *)__va(msm_fb_resources[0].start);
}

void *lge_get_fb_copy_phys_addr(void)
{
	return (void *)fb_copy_phys;
}

void *lge_get_fb_copy_virt_addr(void)
{
	return (void *)fb_copy_virt;
}

static void __init lge_make_fb_pmem(void)
{
	struct membank *bank = &meminfo.bank[0];
	unsigned *temp;

	fb_copy_phys = MSM7X30_EBI0_CS0_BASE + BRYCE_RAM_CONSOLE_SIZE*2;
	fb_copy_size = 480*800*2;
	fb_copy_virt = ioremap(fb_copy_phys, fb_copy_size);

	temp = fb_copy_virt;
	temp = 0x0;

	printk("FB start phys addr : %x\n", fb_copy_phys);
	printk("FB start virt addr : %x\n", fb_copy_virt);
	printk("FB size : %x\n", fb_copy_size);

	return;
}
#endif
/* neo.kang@lge.com	10.12.15. E */

static void __init msm_fb_add_devices(void)
{
/* neo.kang@lge.com	10.12.15. S
 * 0012867 : add the hidden reset
 */ 
#if defined(CONFIG_LGE_HIDDEN_RESET_PATCH)
	lge_make_fb_pmem();
#endif
/* neo.kang@lge.com	10.12.15. E */
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("pmdh", &mddi_pdata);
/* jaeseong.gim 	2010.07.12
                for hdmi 
 */
#ifndef CONFIG_MACH_LGE_BRYCE
	msm_fb_register_device("lcdc", &lcdc_pdata);
#endif
	msm_fb_register_device("dtv", &dtv_pdata);
	//msm_fb_register_device("tvenc", &atv_pdata);

}

static struct msm_panel_common_pdata lcdc_toshiba_panel_data = {
	.gpio_num          = lcd_panel_spi_gpio_num,
};

static struct platform_device lcdc_toshiba_panel_device = {
	.name   = "lcdc_toshiba_wvga",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_toshiba_panel_data,
	}
};

#if defined(CONFIG_MARIMBA_CORE) && \
   (defined(CONFIG_MSM_BT_POWER) || defined(CONFIG_MSM_BT_POWER_MODULE))
static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
	.id     = -1
};

enum {
	BT_RFR,
	BT_CTS,
	BT_RX,
	BT_TX,
};

static struct msm_gpio bt_config_power_on[] = {
	{ GPIO_CFG(134, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
		"UART1DM_RFR" },
	{ GPIO_CFG(135, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
		"UART1DM_CTS" },
	{ GPIO_CFG(136, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
		"UART1DM_Rx" },
	{ GPIO_CFG(137, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
		"UART1DM_Tx" }
};

static struct msm_gpio bt_config_power_off[] = {
	{ GPIO_CFG(134, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
		"UART1DM_RFR" },
	{ GPIO_CFG(135, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
		"UART1DM_CTS" },
	{ GPIO_CFG(136, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
		"UART1DM_Rx" },
	{ GPIO_CFG(137, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
		"UART1DM_Tx" }
};

static struct msm_gpio bt_config_clock[] = {
	{ GPIO_CFG(34, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL,    GPIO_CFG_2MA),
		"BT_REF_CLOCK_ENABLE" },
};
static const char *vregs_bt_marimba_name[] = {
	"s3",
	"s2",
	"gp16",
	"wlan"
};
static struct vreg *vregs_bt_marimba[ARRAY_SIZE(vregs_bt_marimba_name)];

static const char *vregs_bt_bahama_name[] = {
	"s3",
	"usb2",
	"s2",
	"wlan"
};
static struct vreg *vregs_bt_bahama[ARRAY_SIZE(vregs_bt_bahama_name)];

static u8 bahama_version;

static int marimba_bt(int on)
{
	int rc;
	int i;
	struct marimba config = { .mod_id = MARIMBA_SLAVE_ID_MARIMBA };

	struct marimba_config_register {
		u8 reg;
		u8 value;
		u8 mask;
	};

	struct marimba_variant_register {
		const size_t size;
		const struct marimba_config_register *set;
	};

	const struct marimba_config_register *p;

	u8 version;

	const struct marimba_config_register v10_bt_on[] = {
		{ 0xE5, 0x0B, 0x0F },
		{ 0x05, 0x02, 0x07 },
		{ 0x06, 0x88, 0xFF },
		{ 0xE7, 0x21, 0x21 },
		{ 0xE3, 0x38, 0xFF },
		{ 0xE4, 0x06, 0xFF },
	};

	const struct marimba_config_register v10_bt_off[] = {
		{ 0xE5, 0x0B, 0x0F },
		{ 0x05, 0x08, 0x0F },
		{ 0x06, 0x88, 0xFF },
		{ 0xE7, 0x00, 0x21 },
		{ 0xE3, 0x00, 0xFF },
		{ 0xE4, 0x00, 0xFF },
	};

	const struct marimba_config_register v201_bt_on[] = {
		{ 0x05, 0x08, 0x07 },
		{ 0x06, 0x88, 0xFF },
		{ 0xE7, 0x21, 0x21 },
		{ 0xE3, 0x38, 0xFF },
		{ 0xE4, 0x06, 0xFF },
	};

	const struct marimba_config_register v201_bt_off[] = {
		{ 0x05, 0x08, 0x07 },
		{ 0x06, 0x88, 0xFF },
		{ 0xE7, 0x00, 0x21 },
		{ 0xE3, 0x00, 0xFF },
		{ 0xE4, 0x00, 0xFF },
	};

	const struct marimba_config_register v210_bt_on[] = {
		{ 0xE9, 0x01, 0x01 },
		{ 0x06, 0x88, 0xFF },
		{ 0xE7, 0x21, 0x21 },
		{ 0xE3, 0x38, 0xFF },
		{ 0xE4, 0x06, 0xFF },
	};

	const struct marimba_config_register v210_bt_off[] = {
		{ 0x06, 0x88, 0xFF },
		{ 0xE7, 0x00, 0x21 },
		{ 0xE9, 0x00, 0x01 },
		{ 0xE3, 0x00, 0xFF },
		{ 0xE4, 0x00, 0xFF },
	};

	const struct marimba_variant_register bt_marimba[2][4] = {
		{
			{ ARRAY_SIZE(v10_bt_off), v10_bt_off },
			{ 0, NULL },
			{ ARRAY_SIZE(v201_bt_off), v201_bt_off },
			{ ARRAY_SIZE(v210_bt_off), v210_bt_off }
		},
		{
			{ ARRAY_SIZE(v10_bt_on), v10_bt_on },
			{ 0, NULL },
			{ ARRAY_SIZE(v201_bt_on), v201_bt_on },
			{ ARRAY_SIZE(v210_bt_on), v210_bt_on }
		}
	};

	on = on ? 1 : 0;

	rc = marimba_read_bit_mask(&config, 0x11,  &version, 1, 0x1F);
	if (rc < 0) {
		printk(KERN_ERR
			"%s: version read failed: %d\n",
			__func__, rc);
		return rc;
	}

	if ((version >= ARRAY_SIZE(bt_marimba[on])) ||
	    (bt_marimba[on][version].size == 0)) {
		printk(KERN_ERR
			"%s: unsupported version\n",
			__func__);
		return -EIO;
	}

	p = bt_marimba[on][version].set;

	printk(KERN_INFO "%s: found version %d\n", __func__, version);

	for (i = 0; i < bt_marimba[on][version].size; i++) {
		u8 value = (p+i)->value;
		rc = marimba_write_bit_mask(&config,
			(p+i)->reg,
			&value,
			sizeof((p+i)->value),
			(p+i)->mask);
		if (rc < 0) {
			printk(KERN_ERR
				"%s: reg %d write failed: %d\n",
				__func__, (p+i)->reg, rc);
			return rc;
		}
		printk(KERN_INFO "%s: reg 0x%02x value 0x%02x mask 0x%02x\n",
				__func__, (p+i)->reg,
				value, (p+i)->mask);
	}
	return 0;
}

static int bahama_bt(int on)
{
	int rc;
	int i;
	struct marimba config = { .mod_id = SLAVE_ID_BAHAMA };

	struct bahama_variant_register {
		const size_t size;
		const struct bahama_config_register *set;
	};

	const struct bahama_config_register *p;


	const struct bahama_config_register v10_bt_on[] = {
		{ 0xE9, 0x00, 0xFF },
		{ 0xF4, 0x80, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE4, 0x00, 0xFF },
		{ 0xE5, 0x00, 0x0F },
#ifdef CONFIG_WLAN
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0x11, 0x13, 0xFF },
		{ 0xE9, 0x21, 0xFF },
		{ 0x01, 0x0C, 0x1F },
		{ 0x01, 0x08, 0x1F },
	};

	const struct bahama_config_register v20_bt_on_fm_off[] = {
		{ 0x11, 0x0C, 0xFF },
		{ 0x13, 0x01, 0xFF },
		{ 0xF4, 0x80, 0xFF },
		{ 0xF0, 0x00, 0xFF },
		{ 0xE9, 0x00, 0xFF },
#ifdef CONFIG_WLAN
		{ 0x81, 0x00, 0xFF },
		{ 0x82, 0x00, 0xFF },
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0xE9, 0x21, 0xFF }
	};

	const struct bahama_config_register v20_bt_on_fm_on[] = {
		{ 0x11, 0x0C, 0xFF },
		{ 0x13, 0x01, 0xFF },
		{ 0xF4, 0x86, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE9, 0x00, 0xFF },
#ifdef CONFIG_WLAN
		{ 0x81, 0x00, 0xFF },
		{ 0x82, 0x00, 0xFF },
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0xE9, 0x21, 0xFF }
	};

	const struct bahama_config_register v10_bt_off[] = {
		{ 0xE9, 0x00, 0xFF },
	};

	const struct bahama_config_register v20_bt_off_fm_off[] = {
		{ 0xF4, 0x84, 0xFF },
		{ 0xF0, 0x04, 0xFF },
		{ 0xE9, 0x00, 0xFF }
	};

	const struct bahama_config_register v20_bt_off_fm_on[] = {
		{ 0xF4, 0x86, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE9, 0x00, 0xFF }
	};

	const struct bahama_variant_register bt_bahama[2][3] = {
		{
			{ ARRAY_SIZE(v10_bt_off), v10_bt_off },
			{ ARRAY_SIZE(v20_bt_off_fm_off), v20_bt_off_fm_off },
			{ ARRAY_SIZE(v20_bt_off_fm_on), v20_bt_off_fm_on }
		},
		{
			{ ARRAY_SIZE(v10_bt_on), v10_bt_on },
			{ ARRAY_SIZE(v20_bt_on_fm_off), v20_bt_on_fm_off },
			{ ARRAY_SIZE(v20_bt_on_fm_on), v20_bt_on_fm_on }
		}
	};

	u8 offset = 0; /* index into bahama configs */

	/* Init mutex to get/set FM/BT status respectively */
	mutex_init(&config.xfer_lock);

	on = on ? 1 : 0;

	bahama_version = read_bahama_ver();

	if (bahama_version == VER_UNSUPPORTED) {
		dev_err(&msm_bt_power_device.dev,
			"%s: unsupported version\n",
			__func__);
		return -EIO;
	}

	if (bahama_version == VER_2_0) {
		if (marimba_get_fm_status(&config))
			offset = 0x01;
	}

	p = bt_bahama[on][bahama_version + offset].set;

	dev_info(&msm_bt_power_device.dev,
		"%s: found version %d\n", __func__, bahama_version);

	for (i = 0; i < bt_bahama[on][bahama_version + offset].size; i++) {
		u8 value = (p+i)->value;
		rc = marimba_write_bit_mask(&config,
			(p+i)->reg,
			&value,
			sizeof((p+i)->value),
			(p+i)->mask);
		if (rc < 0) {
			dev_err(&msm_bt_power_device.dev,
				"%s: reg %d write failed: %d\n",
				__func__, (p+i)->reg, rc);
			return rc;
		}
		dev_info(&msm_bt_power_device.dev,
			"%s: reg 0x%02x write value 0x%02x mask 0x%02x\n",
				__func__, (p+i)->reg,
				value, (p+i)->mask);
	}
	/* Update BT status */
	if (on)
		marimba_set_bt_status(&config, true);
	else
		marimba_set_bt_status(&config, false);

	/* Destory mutex */
	mutex_destroy(&config.xfer_lock);

	if (bahama_version == VER_2_0 && on) { /* variant of bahama v2 */
		/* Disable s2 as bahama v2 uses internal LDO regulator */
		for (i = 0; i < ARRAY_SIZE(vregs_bt_bahama_name); i++) {
			if (!strcmp(vregs_bt_bahama_name[i], "s2")) {
				rc = vreg_disable(vregs_bt_bahama[i]);
				if (rc < 0) {
					printk(KERN_ERR
						"%s: vreg %s disable "
						"failed (%d)\n",
						__func__,
						vregs_bt_bahama_name[i], rc);
					return -EIO;
				}
				rc = pmapp_vreg_level_vote("BTPW",
								PMAPP_VREG_S2,
								0);
				if (rc < 0) {
					printk(KERN_ERR "%s: vreg "
						"level off failed (%d)\n",
						__func__, rc);
					return -EIO;
				}
				printk(KERN_INFO "%s: vreg disable & "
					"level off successful (%d)\n",
					__func__, rc);
			}
		}
	}

	return 0;
}

static int bluetooth_power_regulators(int on, int bahama_not_marimba)
{
	int i, rc;
	const char **vregs_name;
	struct vreg **vregs;
	int vregs_size;

	if (bahama_not_marimba) {
		vregs_name = vregs_bt_bahama_name;
		vregs = vregs_bt_bahama;
		vregs_size = ARRAY_SIZE(vregs_bt_bahama_name);
	} else {
		vregs_name = vregs_bt_marimba_name;
		vregs = vregs_bt_marimba;
		vregs_size = ARRAY_SIZE(vregs_bt_marimba_name);
	}

	for (i = 0; i < vregs_size; i++) {
		if (bahama_not_marimba && (bahama_version == VER_2_0) &&
			!on && !strcmp(vregs_bt_bahama_name[i], "s2"))
			continue;
		rc = on ? vreg_enable(vregs[i]) :
			  vreg_disable(vregs[i]);
		if (rc < 0) {
			printk(KERN_ERR "%s: vreg %s %s failed (%d)\n",
				__func__, vregs_name[i],
			       on ? "enable" : "disable", rc);
			return -EIO;
		}
	}
	return 0;
}

static int bluetooth_power(int on)
{
	int rc;
	const char *id = "BTPW";

	int bahama_not_marimba = bahama_present();

	if (bahama_not_marimba == -1) {
		printk(KERN_WARNING "%s: bahama_present: %d\n",
				__func__, bahama_not_marimba);
		return -ENODEV;
	}

	if (on) {
		rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);
		if (rc < 0) {
			printk(KERN_ERR "%s: vreg level on failed (%d)\n",
				__func__, rc);
			return rc;
		}

		rc = bluetooth_power_regulators(on, bahama_not_marimba);
		if (rc < 0)
			return -EIO;

		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_ON);
		if (rc < 0)
			return -EIO;

		if (machine_is_msm8x55_svlte_surf() ||
				machine_is_msm8x55_svlte_ffa()) {
					rc = marimba_gpio_config(1);
					if (rc < 0)
						return -EIO;
		}

		rc = (bahama_not_marimba ? bahama_bt(on) : marimba_bt(on));
		if (rc < 0)
			return -EIO;

		msleep(10);

		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_PIN_CTRL);
		if (rc < 0)
			return -EIO;

		if (machine_is_msm8x55_svlte_surf() ||
				machine_is_msm8x55_svlte_ffa()) {
					rc = marimba_gpio_config(0);
					if (rc < 0)
						return -EIO;
		}

		rc = msm_gpios_enable(bt_config_power_on,
			ARRAY_SIZE(bt_config_power_on));

		if (rc < 0)
			return rc;

	} else {
		rc = msm_gpios_enable(bt_config_power_off,
					ARRAY_SIZE(bt_config_power_off));
		if (rc < 0)
			return rc;

		/* check for initial RFKILL block (power off) */
		if (platform_get_drvdata(&msm_bt_power_device) == NULL)
			goto out;

		rc = (bahama_not_marimba ? bahama_bt(on) : marimba_bt(on));
		if (rc < 0)
			return -EIO;

		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_OFF);
		if (rc < 0)
			return -EIO;

		rc = bluetooth_power_regulators(on, bahama_not_marimba);
		if (rc < 0)
			return -EIO;

		if (bahama_version == VER_1_0) {
			rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);
			if (rc < 0) {
				printk(KERN_ERR "%s: vreg level off failed "
				"(%d)\n", __func__, rc);
				return -EIO;
			}
		}
	}

out:
	printk(KERN_DEBUG "Bluetooth power switch: %d\n", on);

	return 0;
}

static void __init bt_power_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(vregs_bt_marimba_name); i++) {
		vregs_bt_marimba[i] = vreg_get(NULL, vregs_bt_marimba_name[i]);
		if (IS_ERR(vregs_bt_marimba[i])) {
			printk(KERN_ERR "%s: vreg get %s failed (%ld)\n",
			       __func__, vregs_bt_marimba_name[i],
			       PTR_ERR(vregs_bt_marimba[i]));
			return;
		}
	}

	for (i = 0; i < ARRAY_SIZE(vregs_bt_bahama_name); i++) {
		vregs_bt_bahama[i] = vreg_get(NULL, vregs_bt_bahama_name[i]);
		if (IS_ERR(vregs_bt_bahama[i])) {
			printk(KERN_ERR "%s: vreg get %s failed (%ld)\n",
			       __func__, vregs_bt_bahama_name[i],
			       PTR_ERR(vregs_bt_bahama[i]));
			return;
		}
	}

    msm_bt_power_device.dev.platform_data = &bluetooth_power;
}
#else
#define bt_power_init(x) do {} while (0)
#endif

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design 	= 2800,
	.voltage_max_design	= 4300,
	.avail_chg_sources   	= AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
};

static struct platform_device msm_batt_device = {
	.name 		    = "msm-battery",
	.id		    = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

static char *msm_adc_surf_device_names[] = {
	"XO_ADC",
};

static struct msm_adc_platform_data msm_adc_pdata;

static struct platform_device msm_adc_device = {
	.name   = "msm_adc",
	.id = -1,
	.dev = {
		.platform_data = &msm_adc_pdata,
	},
};

/* CONFIG_MACH_LGE_BRYCE	ehgrace.kim 10.05.03
	amp device
*/
#ifdef CONFIG_MACH_LGE_BRYCE
#define GPIO_AMP_I2C_SDA	92
#define GPIO_AMP_I2C_SCL	93

static struct gpio_i2c_pin amp_i2c_pin[] = {
	[0] = {
    .sda_pin = GPIO_AMP_I2C_SDA,
    .scl_pin = GPIO_AMP_I2C_SCL,
	},
};

static struct i2c_gpio_platform_data amp_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device amp_i2c_device = {
	.id = 14,
	.name = "i2c-gpio",
	.dev.platform_data = &amp_i2c_pdata,
};

/* ehgrace.kim	10.08.17
	add the audio amps for evb1 and other boards.
*/
/* daniel.kang@lge.com ++ Nov 17 // 0011018: Remove Audience Code */
#if defined (LG_HW_REV1)
static struct i2c_board_info amp_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("wm9093", 0xDC >> 1),
	},
};

/*
#elif defined (LG_HW_REV5) || defined(LG_HW_REV6) //Amp not sharing this I2C in Rev C. kiran.kanneganti@lge.com baikal 0009680
static struct i2c_board_info amp_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("a1026", 0x3E),
	},
};
*/
#elif defined (LG_HW_REV3) || defined(LG_HW_REV4)
static struct i2c_board_info amp_i2c_bdinfo[] = {
/* Sharing the GPIO I2C and add audience device here */
/* revA, revB share GPIO with tpa amp, but audience is not used  */
/* revC, revD : we don't use audience */
//	[0] = {
//		I2C_BOARD_INFO("a1026", 0x3E),
//	},
	[0] = {
		I2C_BOARD_INFO("tpa2055", 0xE0>> 1),
	},
};
/* daniel.kang@lge.com -- Nov 17 // 0011018: Remove Audience Code */

#endif
#endif

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	general gpio-i2c device configuration
*/
#ifdef CONFIG_MACH_LGE_BRYCE
int init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	return 0;
}
#endif 


/* CONFIG_MACH_LGE_BRYCE	ey.cho@lge.com	10.08.20
	hallic driver
*/
#if CONFIG_MACH_LGE_BRYCE
static struct gpio_event_direct_entry bryce_slide_switch_map[] = {
	{ 44,          SW_LID          },
};

static int bryce_gpio_slide_input_func(struct gpio_event_input_devs  *input_dev,
		struct gpio_event_info *info, void **data, int func)
{
	if (func == GPIO_EVENT_FUNC_INIT)
		gpio_tlmm_config(GPIO_CFG(44, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	return gpio_event_input_func(input_dev, info, data, func);
}
static int bryce_gpio_slide_power(
		const struct gpio_event_platform_data *pdata, bool on)
{
	return 0;
}

static struct gpio_event_input_info bryce_slide_switch_info = {
	.info.func = bryce_gpio_slide_input_func,
	.debounce_time.tv64 = 0,
	.flags = 0,
	.type = EV_SW,
	.keymap = bryce_slide_switch_map,
	.keymap_size = ARRAY_SIZE(bryce_slide_switch_map)
};

static struct gpio_event_info *bryce_gpio_slide_info[] = {
	&bryce_slide_switch_info.info,
};

static struct gpio_event_platform_data bryce_gpio_slide_data = {
	.name = "gpio-slide-detect",
	.info = bryce_gpio_slide_info,
	.info_count = ARRAY_SIZE(bryce_gpio_slide_info),
	.power = bryce_gpio_slide_power,
};

static struct platform_device bryce_gpio_slide_device = {
	.name = "gpio-event",
	.id = 0,
	.dev        = {
		.platform_data  = &bryce_gpio_slide_data,
	},
};

static struct platform_device hallic_dock_device = {
	.name   = "hall-ic-dock",
	.id = -1,
	.dev = {
		.platform_data = NULL,
	},
};
#endif
/* CONFIG_MACH_LGE_BRYCE	kwangdo.yi@lge.com	10.10.8
   0010357: add ram dump for ERS in AP side.
*/
#if CONFIG_MACH_LGE_BRYCE
#ifdef CONFIG_ANDROID_RAM_CONSOLE
/* neo.kang@lge.com	10.12.15. 
 * 0012867 : the definition is moved to board_lge.h */

static struct resource ram_console_resource[] = {
   {
      .name = "ram_console",
      .start = BRYCE_RAM_CONSOLE_BASE,
      .end = BRYCE_RAM_CONSOLE_BASE + BRYCE_RAM_CONSOLE_SIZE - 1,
      .flags = IORESOURCE_MEM,
   }
};

static struct platform_device ram_console_device = {
   .name = "ram_console",
   .id = -1,
   .num_resources = ARRAY_SIZE(ram_console_resource),
   .resource = ram_console_resource,
};
#endif
/* kwangdo.yi 10.10.8 E */

/* kwangdo.yi 10.10.26 S
0010357: add ram dump for ERS in AP side
 */
static struct platform_device ers_kernel = {
	.name = "ers-kernel",
};
#endif
/* kwangdo.yi 10.10.26 E */

// BEGIN 0011665: eundeok.bae@lge.com FTM MODE FOR ONLY KERNEL BOOTING
// [KERNEL] Added source code For Sleep Mode Test, Test Mode V8.3 [250-42] 
// LGE_CHANGE [dojip.kim@lge.com] 2010-09-28
static struct platform_device testmode_device = {
	.name = "testmode",
	.id = -1,
};
// END 0011665: eundeok.bae@lge.com FTM MODE FOR ONLY KERNEL BOOTING

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	welcome new board.
*/
#if CONFIG_MACH_LGE_BRYCE
static struct platform_device *lge_devices[] __initdata = {
#if defined(CONFIG_SERIAL_MSM) || defined(CONFIG_MSM_SERIAL_DEBUGGER)
#ifdef LG_HW_REV1
	&msm_device_uart3,
#endif
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	&msm_device_uart2,
#endif
#endif
	&msm_device_smd,
	&msm_device_dmov,
	&msm_device_nand,
#ifdef CONFIG_USB_MSM_OTG_72K
	&msm_device_otg,
#ifdef CONFIG_USB_GADGET
	&msm_device_gadget_peripheral,
#endif
#endif
#ifdef CONFIG_LGE_USB_GADGET_NDIS_DRIVER
    &ecm_device,
#endif
/*	CONFIG_USB_ANDROID	yk.kim@lge.com	10.09.14
	support Android USB Gadget Driver.	*/
#ifdef CONFIG_USB_ANDROID
/* [yk.kim@lge.com] 2010-12-28, ums sysfs enable */
#if 1
	&usb_mass_storage_device,
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	&usb_diag_device,
#endif
	&android_usb_device,
#endif
	//&qsd_device_spi,
#ifdef CONFIG_I2C_SSBI
	&msm_device_ssbi6,
	&msm_device_ssbi7,
#endif
	&android_pmem_device,
	&msm_fb_device,
	&msm_migrate_pages_device,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&android_pmem_kernel_ebi1_device,
	&android_pmem_adsp_device,
	&android_pmem_audio_device,
	&msm_device_i2c,
	&msm_device_i2c_2,
	&msm_device_uart_dm1,
	&hs_device,
#ifdef CONFIG_MSM7KV2_AUDIO
	&msm_aictl_device,
	&msm_mi2s_device,
	&msm_lpa_device,
	&msm_aux_pcm_device,
#endif
	&msm_device_adspdec,
#if !defined(LG_HW_REV1) && !defined(LG_HW_REV2)	
	&qup_device_i2c,
#endif
#if defined(CONFIG_MARIMBA_CORE) && \
   (defined(CONFIG_MSM_BT_POWER) || defined(CONFIG_MSM_BT_POWER_MODULE))
	//&msm_bt_power_device,
#endif
	&msm_device_kgsl,
	&msm_device_vidc_720p,
#ifdef CONFIG_MSM_GEMINI
	&msm_gemini_device,
#endif
#ifdef CONFIG_MSM_VPE
	&msm_vpe_device,
#endif
	&msm_batt_device,
	&msm_adc_device,
	
	&msm_ebi0_thermal,
	&msm_ebi1_thermal,

	&mddi_lgit_device,
	&amp_i2c_device,	

#if defined(LG_HW_REV2)
	&tskey_i2c_device,
#endif
#if defined(LG_HW_REV1) || defined(LG_HW_REV2) 
	&amri5k_spi_device,
#endif 	
/* CONFIG_MACH_LGE_BRYCE	kwangdo.yi@lge.com	10.10.8
	0010357: add ram dump for ERS in AP side
*/
#ifdef CONFIG_ANDROID_RAM_CONSOLE
   &ram_console_device,
#endif
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	&bl_i2c_device,
	&sensor_i2c_device,
#endif 
#if defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	&hallic_dock_device,
#endif 

// BEGIN: 0009214 sehyuny.kim@lge.com 2010-09-03
// MOD 0009214: [DIAG] LG Diag feature added in side of android
	&lg_fw_diagcmd_device,	
	&lg_diag_cmd_device,
// END: 0009214 sehyuny.kim@lge.com 2010-09-03

/* kwangdo.yi 10.10.26 S
   0010357: add ram dump for ERS in AP side
 */
 #if CONFIG_MACH_LGE_BRYCE
	&ers_kernel,
#endif	
/* kwangdo.yi 10.10.26 E */

// BEGIN 0011665: eundeok.bae@lge.com FTM MODE FOR ONLY KERNEL BOOTING
// [KERNEL] Added source code For Sleep Mode Test, Test Mode V8.3 [250-42] 
// LGE_CHANGE [dojip.kim@lge.com] 2010-09-28
	&testmode_device,
// END 0011665: eundeok.bae@lge.com FTM MODE FOR ONLY KERNEL BOOTING
};
#endif

static struct msm_gpio msm_i2c_gpios_hw[] = {
	{ GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_scl" },
	{ GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_sda" },
};

static struct msm_gpio msm_i2c_gpios_io[] = {
	{ GPIO_CFG(70, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_scl" },
	{ GPIO_CFG(71, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_sda" },
};

static struct msm_gpio qup_i2c_gpios_io[] = {
	{ GPIO_CFG(16, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_scl" },
	{ GPIO_CFG(17, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_sda" },
};
static struct msm_gpio qup_i2c_gpios_hw[] = {
	{ GPIO_CFG(16, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_scl" },
	{ GPIO_CFG(17, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_sda" },
};

static void
msm_i2c_gpio_config(int adap_id, int config_type)
{
	struct msm_gpio *msm_i2c_table;

	/* Each adapter gets 2 lines from the table */
	if (adap_id > 0)
		return;
	if (config_type)
		msm_i2c_table = &msm_i2c_gpios_hw[adap_id*2];
	else
		msm_i2c_table = &msm_i2c_gpios_io[adap_id*2];
	msm_gpios_enable(msm_i2c_table, 2);
}
/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
static struct vreg *qup_vreg;
#endif
static void
qup_i2c_gpio_config(int adap_id, int config_type)
{
	int rc = 0;
	struct msm_gpio *qup_i2c_table;
	/* Each adapter gets 2 lines from the table */
	if (adap_id != 4)
		return;
	if (config_type)
		qup_i2c_table = qup_i2c_gpios_hw;
	else
		qup_i2c_table = qup_i2c_gpios_io;
	rc = msm_gpios_enable(qup_i2c_table, 2);
	if (rc < 0)
		printk(KERN_ERR "QUP GPIO enable failed: %d\n", rc);
	/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
	if (qup_vreg) {
		int rc = vreg_set_level(qup_vreg, 1800);
		if (rc) {
			pr_err("%s: vreg LVS1 set level failed (%d)\n",
			__func__, rc);
		}
		rc = vreg_enable(qup_vreg);
		if (rc) {
			pr_err("%s: vreg_enable() = %d \n",
			__func__, rc);
		}
	}
#endif
}

static struct msm_i2c_platform_data msm_i2c_pdata = {
	.clk_freq = 100000,
	.pri_clk = 70,
	.pri_dat = 71,
	.rmutex  = 1,
	.rsl_id = "D:I2C02000021",
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_init(void)
{
	if (msm_gpios_request(msm_i2c_gpios_hw, ARRAY_SIZE(msm_i2c_gpios_hw)))
		pr_err("failed to request I2C gpios\n");

	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

static struct msm_i2c_platform_data msm_i2c_2_pdata = {
	.clk_freq = 100000,
	.rmutex  = 1,
	.rsl_id = "D:I2C02000022",
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_2_init(void)
{
	msm_device_i2c_2.dev.platform_data = &msm_i2c_2_pdata;
}

static struct msm_i2c_platform_data qup_i2c_pdata = {
	.clk_freq = 400000,
	.pclk = "camif_pad_pclk",
	.msm_i2c_config_gpio = qup_i2c_gpio_config,
};

static void __init qup_device_i2c_init(void)
{
	if (msm_gpios_request(qup_i2c_gpios_hw, ARRAY_SIZE(qup_i2c_gpios_hw)))
		pr_err("failed to request I2C gpios\n");

	qup_device_i2c.dev.platform_data = &qup_i2c_pdata;
	/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
	qup_vreg = vreg_get(NULL, "lvsw1");
	if (IS_ERR(qup_vreg)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(qup_vreg));
	}
#endif
}

#ifdef CONFIG_I2C_SSBI
static struct msm_ssbi_platform_data msm_i2c_ssbi6_pdata = {
	.rsl_id = "D:PMIC_SSBI",
	.controller_type = MSM_SBI_CTRL_SSBI2,
};

static struct msm_ssbi_platform_data msm_i2c_ssbi7_pdata = {
	.rsl_id = "D:CODEC_SSBI",
	.controller_type = MSM_SBI_CTRL_SSBI,
};
#endif

static struct msm_acpu_clock_platform_data msm7x30_clock_data = {
	.acpu_switch_time_us = 50,
	.vdd_switch_time_us = 62,
};

static void __init msm7x30_init_irq(void)
{
	msm_init_irq();
}

static struct msm_gpio msm_nand_ebi2_cfg_data[] = {
	{GPIO_CFG(86, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "ebi2_cs1"},
	{GPIO_CFG(115, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "ebi2_busy1"},
};

struct vreg *vreg_s3;
struct vreg *vreg_mmc;

#if (defined(CONFIG_MMC_MSM_SDC1_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC3_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC4_SUPPORT))

struct sdcc_gpio {
	struct msm_gpio *cfg_data;
	uint32_t size;
	struct msm_gpio *sleep_cfg_data;
};
#if defined(CONFIG_MMC_MSM_SDC1_SUPPORT)
static struct msm_gpio sdc1_lvlshft_cfg_data[] = {
	{GPIO_CFG(35, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA), "sdc1_lvlshft"},
};
#endif
static struct msm_gpio sdc1_cfg_data[] = {

/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.08.09
	For LTE SDIO Driver
*/
#ifdef  CONFIG_MACH_LGE_BRYCE
/* jaegyu lee */	
#ifdef CONFIG_LTE
	{GPIO_CFG(GPIO_LTE_SDIO_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc2_clk"},
	{GPIO_CFG(GPIO_LTE_SDIO_CMD, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_cmd"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_3, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_3"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_2, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_2"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_1, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_1"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_0, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_0"},
#else
	{GPIO_CFG(38, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc1_clk"},
	{GPIO_CFG(39, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_cmd"},
	{GPIO_CFG(40, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_3"},
	{GPIO_CFG(41, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_2"},
	{GPIO_CFG(42, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_1"},
	{GPIO_CFG(43, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_0"},
#endif
/* jaegyu lee */
#endif //CONFIG_MACH_LGE_BRYCE

};
/* BEGIN: 0014992 jaegyu.lee@lge.com 2011-01-28 */
/* MOD 0014992: [LTE] At AP power collapse, Enable SDIO Pin set to GPIO input and Pull Up */
static struct msm_gpio sdc1_sleep_cfg_data[] = {
	{GPIO_CFG(GPIO_LTE_SDIO_CLK, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "sdc1_clk"},
	{GPIO_CFG(GPIO_LTE_SDIO_CMD, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "sdc1_cmd"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_3, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "sdc1_dat_3"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_2, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "sdc1_dat_2"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_1, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "sdc1_dat_1"},
	{GPIO_CFG(GPIO_LTE_SDIO_DATA_0, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "sdc1_dat_0"},
};
/* END: 0014992 jaegyu.lee@lge.com 2011-01-28 */ 
static struct msm_gpio sdc2_cfg_data[] = {
	{GPIO_CFG(64, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc2_clk"},
	{GPIO_CFG(65, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_cmd"},
	{GPIO_CFG(66, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_3"},
	{GPIO_CFG(67, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_2"},
	{GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_1"},
	{GPIO_CFG(69, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_0"},

#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	{GPIO_CFG(115, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_4"},
	{GPIO_CFG(114, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_5"},
	{GPIO_CFG(113, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_6"},
	{GPIO_CFG(112, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_7"},
#endif
};

static struct msm_gpio sdc3_cfg_data[] = {
	{GPIO_CFG(110, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc3_clk"},
	{GPIO_CFG(111, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_cmd"},
	{GPIO_CFG(116, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_3"},
	{GPIO_CFG(117, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_2"},
	{GPIO_CFG(118, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_1"},
	{GPIO_CFG(119, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_0"},
};

static struct msm_gpio sdc3_sleep_cfg_data[] = {
	{GPIO_CFG(110, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_clk"},
	{GPIO_CFG(111, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_cmd"},
	{GPIO_CFG(116, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_3"},
	{GPIO_CFG(117, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_2"},
	{GPIO_CFG(118, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_1"},
	{GPIO_CFG(119, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_0"},
};

static struct msm_gpio sdc4_cfg_data[] = {
	{GPIO_CFG(58, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc4_clk"},
	{GPIO_CFG(59, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_cmd"},
	{GPIO_CFG(60, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_3"},
	{GPIO_CFG(61, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_2"},
	{GPIO_CFG(62, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_1"},
	{GPIO_CFG(63, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_0"},
};

static struct sdcc_gpio sdcc_cfg_data[] = {
	{
		.cfg_data = sdc1_cfg_data,
		.size = ARRAY_SIZE(sdc1_cfg_data),
/* BEGIN: 0014992 jaegyu.lee@lge.com 2011-01-28 */
/* MOD 0014992: [LTE] At AP power collapse, Enable SDIO Pin set to GPIO input and Pull Up */
		.sleep_cfg_data = sdc1_sleep_cfg_data,
/* END: 0014992 jaegyu.lee@lge.com 2011-01-28 */ 
	},
	{
		.cfg_data = sdc2_cfg_data,
		.size = ARRAY_SIZE(sdc2_cfg_data),
		.sleep_cfg_data = NULL,
	},
	{
		.cfg_data = sdc3_cfg_data,
		.size = ARRAY_SIZE(sdc3_cfg_data),
		.sleep_cfg_data = sdc3_sleep_cfg_data,
	},
	{
		.cfg_data = sdc4_cfg_data,
		.size = ARRAY_SIZE(sdc4_cfg_data),
		.sleep_cfg_data = NULL,
	},
};

struct sdcc_vreg {
	struct vreg *vreg_data;
	unsigned level;
};

static struct sdcc_vreg sdcc_vreg_data[4];

static unsigned long vreg_sts, gpio_sts;

static uint32_t msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_gpio *curr;

	curr = &sdcc_cfg_data[dev_id - 1];

	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return rc;

	if (enable) {
		set_bit(dev_id, &gpio_sts);
		rc = msm_gpios_request_enable(curr->cfg_data, curr->size);
		if (rc)
			printk(KERN_ERR "%s: Failed to turn on GPIOs for slot %d\n",
				__func__,  dev_id);
	} else {
		clear_bit(dev_id, &gpio_sts);
		if (curr->sleep_cfg_data) {
			msm_gpios_enable(curr->sleep_cfg_data, curr->size);
			msm_gpios_free(curr->sleep_cfg_data, curr->size);
		} else {
			msm_gpios_disable_free(curr->cfg_data, curr->size);
		}
	}

	return rc;
}

static uint32_t msm_sdcc_setup_vreg(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_vreg *curr;
	static int enabled_once[] = {0, 0, 0, 0};

	curr = &sdcc_vreg_data[dev_id - 1];

	if (!(test_bit(dev_id, &vreg_sts)^enable))
		return rc;

	if (!enable || enabled_once[dev_id - 1])
		return 0;

	if (enable) {
		set_bit(dev_id, &vreg_sts);
		rc = vreg_set_level(curr->vreg_data, curr->level);
		if (rc) {
			printk(KERN_ERR "%s: vreg_set_level() = %d \n",
					__func__, rc);
		}
		rc = vreg_enable(curr->vreg_data);
		if (rc) {
			printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		}
		enabled_once[dev_id - 1] = 1;
	} else {
		clear_bit(dev_id, &vreg_sts);
		rc = vreg_disable(curr->vreg_data);
		if (rc) {
			printk(KERN_ERR "%s: vreg_disable() = %d \n",
					__func__, rc);
		}
	}
	return rc;
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	rc = msm_sdcc_setup_gpio(pdev->id, (vdd ? 1 : 0));
	if (rc)
		goto out;

	if (pdev->id == 4) /* S3 is always ON and cannot be disabled */
		rc = msm_sdcc_setup_vreg(pdev->id, (vdd ? 1 : 0));
out:
	return rc;
}
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int msm7x30_sdcc_slot_status(struct device *dev)
{
#ifdef CONFIG_MACH_LGE_BRYCE
	printk("gpio_get_value(SYS_GPIO_SD_DET) returns %d\n", gpio_get_value(SYS_GPIO_SD_DET));
	return (unsigned int) gpio_get_value(SYS_GPIO_SD_DET)?0:1;
#else
	return (unsigned int)
		gpio_get_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SD_DET - 1));
#endif 	
}
#endif

/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.08.09
	For LTE SDIO Driver
*/
#ifdef  CONFIG_MACH_LGE_BRYCE
/* jaegyu lee */
/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.10.06
	Modification for using LTE reset GPIO to Slot rescan 
*/
#ifdef CONFIG_LTE
static unsigned int msm_sdcc_lte_slot_status(struct device *dev)
{
	return gpio_get_value(GPIO_LTE_SDIO_RESET);
}
#endif
/* jaegyu lee */
#endif //CONFIG_MACH_LGE_BRYCE

static int msm_sdcc_get_wpswitch(struct device *dv)
{
	void __iomem *wp_addr = 0;
	uint32_t ret = 0;
	struct platform_device *pdev;

/*CONFIG_MACH_LGE_BRYCE sungmin.shin	10.07.19
	lge_bryce doesn't use WP status
*/
#ifdef CONFIG_MACH_LGE_BRYCE
	return -1;
#else
	if (!(machine_is_msm7x30_surf()))
		return -1;
#endif 

	pdev = container_of(dv, struct platform_device, dev);

	wp_addr = ioremap(FPGA_SDCC_STATUS, 4);
	if (!wp_addr) {
		pr_err("%s: Could not remap %x\n", __func__, FPGA_SDCC_STATUS);
		return -ENOMEM;
	}

	ret = (((readl(wp_addr) >> 4) >> (pdev->id-1)) & 0x01);
	pr_info("%s: WP Status for Slot %d = 0x%x \n", __func__,
							pdev->id, ret);
	iounmap(wp_addr);

	return ret;
}
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT

/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.08.09
	For LTE SDIO Driver
*/
/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.10.06
	Modification for using LTE reset GPIO to Slot rescan 
*/
#ifdef  CONFIG_MACH_LGE_BRYCE
/* jaegyu lee */
#ifdef CONFIG_LTE
static struct mmc_platform_data msm7x30_sdcc_lte_data = {
	.ocr_mask = MMC_VDD_27_28 | MMC_VDD_28_29,
//	.ocr_mask = MMC_VDD_30_31, /* old setting */
	.translate_vdd = msm_sdcc_setup_power,
	.mmc_bus_width = MMC_CAP_4_BIT_DATA,
	.status = msm_sdcc_lte_slot_status,
	.status_irq = MSM_GPIO_TO_INT(GPIO_LTE_SDIO_RESET),
	.irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wpswitch = msm_sdcc_get_wpswitch, /* what's this?? */
	.dummy52_required = 1,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#else
static struct mmc_platform_data msm7x30_sdc1_data = {
	.ocr_mask	= MMC_VDD_165_195,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDC1_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#endif
/* jaegyu lee */
#endif //CONFIG_MACH_LGE_BRYCE

#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct mmc_platform_data msm7x30_sdc2_data = {
	.ocr_mask	= MMC_VDD_165_195 | MMC_VDD_27_28,
	.translate_vdd	= msm_sdcc_setup_power,
#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
#ifdef CONFIG_MMC_MSM_SDC2_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 1,
};
#endif

/* neo.kang@lge.com */
//#if defined(CONFIG_LGE_BCM432X_PATCH)
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
#if defined(CONFIG_LGE_BCM432X_PATCH)

static unsigned int bcm432x_sdcc_wlan_slot_status(struct device *dev)
{
	return gpio_get_value(CONFIG_BCM4329_GPIO_WL_RESET);
}
static struct mmc_platform_data bcm432x_sdcc_wlan_data = {
	.ocr_mask	= MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.status 		= bcm432x_sdcc_wlan_slot_status,
	.status_irq 	= MSM_GPIO_TO_INT(CONFIG_BCM4329_GPIO_WL_RESET),
	.irq_flags 		= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDC3_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#else
static struct mmc_platform_data msm7x30_sdc3_data = {
	.ocr_mask	= MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	.sdiowakeup_irq = MSM_GPIO_TO_INT(118),
#endif
#ifdef CONFIG_MMC_MSM_SDC3_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 1,
};
#endif
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct mmc_platform_data msm7x30_sdc4_data = {
	.ocr_mask	= MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status      = msm7x30_sdcc_slot_status,
/*sungmin.shin, sungwoo.cho	10.07.02
	GPIO card detect
*/
#ifdef CONFIG_MACH_LGE_BRYCE
	.status_irq  = MSM_GPIO_TO_INT(SYS_GPIO_SD_DET),
#else	
	.status_irq  = PM8058_GPIO_IRQ(PMIC8058_IRQ_BASE, PMIC_GPIO_SD_DET - 1),
#endif	
	.irq_flags   = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
	.wpswitch    = msm_sdcc_get_wpswitch,
#ifdef CONFIG_MMC_MSM_SDC4_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static void msm_sdc1_lvlshft_enable(void)
{
	int rc;

	/* Enable LDO5, an input to the FET that powers slot 1 */
	rc = vreg_set_level(vreg_mmc, 2850);
	if (rc)
		printk(KERN_ERR "%s: vreg_set_level() = %d \n",	__func__, rc);

	rc = vreg_enable(vreg_mmc);
	if (rc)
		printk(KERN_ERR "%s: vreg_enable() = %d \n", __func__, rc);

	/* Enable GPIO 35, to turn on the FET that powers slot 1 */
	rc = msm_gpios_request_enable(sdc1_lvlshft_cfg_data,
				ARRAY_SIZE(sdc1_lvlshft_cfg_data));
	if (rc)
		printk(KERN_ERR "%s: Failed to enable GPIO 35\n", __func__);

	rc = gpio_direction_output(GPIO_PIN(sdc1_lvlshft_cfg_data[0].gpio_cfg),
				1);
	if (rc)
		printk(KERN_ERR "%s: Failed to turn on GPIO 35\n", __func__);
}
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static void sdio_wakeup_gpiocfg_slot3(void)
{
	gpio_request(118, "sdio_wakeup");
	gpio_direction_output(118, 1);
	/*
	 * MSM GPIO 118 will be used as both SDIO wakeup irq and
	 * DATA_1 for slot 2. Hence, leave it to SDCC driver to
	 * request this gpio again when it wants to use it as a
	 * data line.
	 */
	gpio_free(118);
}
#endif

static void __init msm7x30_init_mmc(void)
{
	int rc;
#if defined(LG_HW_REV2) || defined(LG_HW_REV3)	
	struct vreg *vreg_test;
#endif

#ifdef CONFIG_MACH_LGE_BRYCE
	gpio_tlmm_config(GPIO_CFG(SYS_GPIO_SD_DET, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	printk("%s: gpio_get_value(SYS_GPIO_SD_DET) returns %d\n", __func__, gpio_get_value(SYS_GPIO_SD_DET));

	vreg_s3 = vreg_get(NULL, "s3");
	if (IS_ERR(vreg_s3)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
					 __func__, PTR_ERR(vreg_s3));
		return;
	}

/*sungmin.shin	10.07.02
	EVB1 board
	SDCC1 = NULL (GPIO38)
	SDCC2 = L2K (GPIO64)
	SDCC3 = BT/WLAN (GPIO110)
	SDCC4 = SDCard (GPIO58)

	EVB2 board, Rev.A board
	SDCC1 = L2000 (GPIO38)
	SDCC2 = eMMC (GPIO64)
	SDCC3 = BT/WLAN (GPIO110)
	SDCC4 = SDCard (GPIO58)	
*/
#ifdef LG_HW_REV1
	vreg_mmc = vreg_get(NULL, "gp4");
#endif 

#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	vreg_mmc = vreg_get(NULL, "mmc"); //HW L5
#endif 

	if (IS_ERR(vreg_mmc)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
					 __func__, PTR_ERR(vreg_mmc));
		return;
	}

/*sungmin.shin	10.07.15
	EVB2 board workaround. 
*/
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) 
	vreg_test = vreg_get(NULL, "gp7"); //HW L8, VREG_SENSOR_IO_1.8V
	vreg_set_level(vreg_test, 1800);
	gpio_tlmm_config(GPIO_CFG(SYS_GPIO_SD_EN_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_ENABLE); 
	gpio_set_value(SYS_GPIO_SD_EN_N, 0);
#endif

/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.08.09
	For LTE SDIO Driver
*/
#ifdef  CONFIG_MACH_LGE_BRYCE
/* jaegyu lee */
/* daeok kim Start*/
/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.10.06
	Modification for using LTE reset GPIO to Slot rescan 
*/
#ifdef CONFIG_LTE
	if (gpio_request(GPIO_LTE_SDIO_RESET, "sdc2_status_irq"))
		pr_err("failed to request gpio sdc2_status_irq\n");

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_LTE_SDIO_RESET, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO GPIO_LTE_SDIO_RESET%d\n", __func__, rc);
/* kwangdo.yi@lge.com S 2010.09.04
	replaced with gpio_direction_output to fix build err
*/
#if 0
	gpio_configure(GPIO_LTE_SDIO_RESET, GPIOF_DRIVE_OUTPUT);
#else
	gpio_direction_output(GPIO_LTE_SDIO_RESET,0);
#endif
	/* kwangdo.yi@lge.com E 2010.09.04
		comment out to fix build err
		
	*/

	gpio_set_value(GPIO_LTE_SDIO_RESET, 0);	
#endif
/* daeok kim End*/
/* jaegyu lee */
#endif //CONFIG_MACH_LGE_BRYCE

/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */
	if (gpio_request(GPIO_L2K_HOST_WAKEUP, "l2k_host_wakeup"))
		pr_err("failed to request gpio l2k_host_wakeup\n");
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_L2K_HOST_WAKEUP, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO HOST_WAKEUP%d\n", __func__, rc);
	gpio_direction_input(GPIO_L2K_HOST_WAKEUP);

	if (gpio_request(GPIO_L2K_LTE_STATUS, "l2k_lte_status"))
		pr_err("failed to request gpio l2k_lte_status\n");
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_L2K_LTE_STATUS, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO LTE_STATUS%d\n", __func__, rc);
	gpio_direction_input(GPIO_L2K_LTE_STATUS);

	if (gpio_request(GPIO_L2K_HOST_STATUS, "l2k_host_status"))
		pr_err("failed to request gpio l2k_host_status\n");
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_L2K_HOST_STATUS, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO HOST_STATUS%d\n", __func__, rc);
	gpio_direction_output(GPIO_L2K_HOST_STATUS,0);
	gpio_set_value(GPIO_L2K_HOST_STATUS, 0);	
	
	if (gpio_request(GPIO_L2K_LTE_WAKEUP, "l2k_lte_wakeup"))
		pr_err("failed to request gpio l2k_lte_wakeup\n");
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_L2K_LTE_WAKEUP, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO LTE_WAKEUP%d\n", __func__, rc);
	gpio_direction_output(GPIO_L2K_LTE_WAKEUP,0);
	gpio_set_value(GPIO_L2K_LTE_WAKEUP, 0);	
/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   	
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT

/* CONFIG_LTE	daeok.kim	10.08.13
	Blocking this code For LTE SDIO Driver
*/

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

	sdcc_vreg_data[0].vreg_data = vreg_s3;
	sdcc_vreg_data[0].level = 1800;

/* CONFIG_MACH_LGE_BRYCE	daeok.kim	10.08.09
	For LTE SDIO Driver
*/
#ifdef  CONFIG_MACH_LGE_BRYCE
/* jaegyu lee */
#ifdef CONFIG_LTE
	msm_add_sdcc(1, &msm7x30_sdcc_lte_data);	
#else
	msm_add_sdcc(1, &msm7x30_sdc1_data);
#endif
/* jaegyu lee */
#endif //CONFIG_MACH_LGE_BRYCE

#endif
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	if (machine_is_msm8x55_svlte_surf())
		msm7x30_sdc2_data.msmsdcc_fmax =  24576000;
	sdcc_vreg_data[1].vreg_data = vreg_s3;
	sdcc_vreg_data[1].level = 1800;
	msm_add_sdcc(2, &msm7x30_sdc2_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	sdcc_vreg_data[2].vreg_data = vreg_s3;
	sdcc_vreg_data[2].level = 1800;
	msm_sdcc_setup_gpio(3, 1);
#if defined(CONFIG_LGE_BCM432X_PATCH)
	printk(KERN_ERR "[dongp.kim@lge.com] CONFIG_BCM4329_GPIO_WL_HOSTWAKEUP:%d\n", CONFIG_BCM4329_GPIO_WL_HOSTWAKEUP);
	msm_add_sdcc(3, &bcm432x_sdcc_wlan_data);
#else 
	msm_add_sdcc(3, &msm7x30_sdc3_data);
#endif
#endif
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	sdcc_vreg_data[3].vreg_data = vreg_mmc;
	sdcc_vreg_data[3].level = 2850;
	msm_add_sdcc(4, &msm7x30_sdc4_data);
#endif	
#endif 
}

static void __init msm7x30_init_nand(void)
{
	char *build_id;
	struct flash_platform_data *plat_data;

	build_id = socinfo_get_build_id();
	if (build_id == NULL) {
		pr_err("%s: Build ID not available from socinfo\n", __func__);
		return;
	}

	if (build_id[8] == 'C' &&
			!msm_gpios_request_enable(msm_nand_ebi2_cfg_data,
			ARRAY_SIZE(msm_nand_ebi2_cfg_data))) {
		plat_data = msm_device_nand.dev.platform_data;
		plat_data->interleave = 1;
		printk(KERN_INFO "%s: Interleave mode Build ID found\n",
			__func__);
	}
}

#ifdef CONFIG_SERIAL_MSM_CONSOLE
static struct msm_gpio uart2_config_data[] = {
/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.15
	GPIO49, 50 being used as other purpose.
*/
#if CONFIG_MACH_LGE_BRYCE
#else
	{ GPIO_CFG(49, 2, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART2_RFR"},
	{ GPIO_CFG(50, 2, GPIO_CFG_INPUT,   GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART2_CTS"},
#endif	
	{ GPIO_CFG(51, 2, GPIO_CFG_INPUT,   GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART2_Rx"},
	{ GPIO_CFG(52, 2, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART2_Tx"},
};

static void msm7x30_init_uart2(void)
{
	msm_gpios_request_enable(uart2_config_data,
			ARRAY_SIZE(uart2_config_data));

}

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	UART3 settings
*/
#if CONFIG_MACH_LGE_BRYCE
static struct msm_gpio uart3_config_data[] = {
	{ GPIO_CFG(53, 1, GPIO_CFG_INPUT,   GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART3_Rx"},
	{ GPIO_CFG(54, 1, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART3_Tx"},
	//{ GPIO_CFG(56, 2, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART3_RFR"},
	//{ GPIO_CFG(55, 2, GPIO_CFG_INPUT,   GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART3_CTS"},	
};

static void msm7x30_init_uart3(void)
{
	msm_gpios_request_enable(uart3_config_data,
			ARRAY_SIZE(uart3_config_data));
}
#endif

#endif

/* TSIF begin */
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)

#define TSIF_B_SYNC      GPIO_CFG(37, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_DATA      GPIO_CFG(36, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_EN        GPIO_CFG(35, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_CLK       GPIO_CFG(34, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)

static const struct msm_gpio tsif_gpios[] = {
	{ .gpio_cfg = TSIF_B_CLK,  .label =  "tsif_clk", },
	{ .gpio_cfg = TSIF_B_EN,   .label =  "tsif_en", },
	{ .gpio_cfg = TSIF_B_DATA, .label =  "tsif_data", },
	{ .gpio_cfg = TSIF_B_SYNC, .label =  "tsif_sync", },
};

static struct msm_tsif_platform_data tsif_platform_data = {
	.num_gpios = ARRAY_SIZE(tsif_gpios),
	.gpios = tsif_gpios,
	.tsif_pclk = "tsif_pclk",
	.tsif_ref_clk = "tsif_ref_clk",
};
#endif /* defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE) */
/* TSIF end   */

static void __init pmic8058_leds_init(void)
{
/* 	jihye.ahn	10.08.18
	keyboard-backlight & RGB indicator
*/
/* jihye.ahn@lge.com   10-09-20   add Rev.C board feature */
#if defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
		pm8058_7x30_data.sub_devices[PM8058_SUBDEV_LED].platform_data
			= &pm8058_surf_leds_data;
		pm8058_7x30_data.sub_devices[PM8058_SUBDEV_LED].data_size
			= sizeof(pm8058_surf_leds_data);
#endif
/* jihye.ahn@lge.com   10-09-30   to enable RGB indicator */
/* removed surf and fluid code */
}

/* CONFIG_MACH_LGE_BRYCE	hyunjong.do@lge.com	10.09.21
 *	special clock setting for wifi and bluetooth 
 *  add cable type function 
 */
#if CONFIG_MACH_LGE_BRYCE
static void pm8058_special_clock0_setting (void)
{
	unsigned on =1;
	int fn_type = CUSTOMER_CMD1_SET_SPECIAL_CLOCK0;

	printk("[LGE_PWR] pm8058_special_clock0_setting \n");
	msm_proc_comm(PCOM_CUSTOMER_CMD1,  &on,&fn_type);
	return;
}

int get_msm_cable_type(void)
{ 
	unsigned int cable_type;
	int fn_type = CUSTOMER_CMD1_GET_CABLE_TYPE;

	msm_proc_comm(PCOM_CUSTOMER_CMD1,  &cable_type,&fn_type);
	printk("[LGE_PWR] cable type detection from muic at modem side Cable=%d \n",cable_type);

	return cable_type;
}
#endif


static struct msm_spm_platform_data msm_spm_data __initdata = {
	.reg_base_addr = MSM_SAW_BASE,

	.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x05,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x18,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0x00006666,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0xFF000666,

	.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x01,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x03,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

	.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

	.awake_vlevel = 0xF2,
	.retention_vlevel = 0xE0,
	.collapse_vlevel = 0x72,
	.retention_mid_vlevel = 0xE0,
	.collapse_mid_vlevel = 0xE0,

	.vctl_timeout_us = 50,
};

#if defined(CONFIG_TOUCHSCREEN_TSC2007) || \
	defined(CONFIG_TOUCHSCREEN_TSC2007_MODULE)

#define TSC2007_TS_PEN_INT	20

static struct msm_gpio tsc2007_config_data[] = {
	{ GPIO_CFG(TSC2007_TS_PEN_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	"tsc2007_irq" },
};

static struct vreg *vreg_tsc_s3;
static struct vreg *vreg_tsc_s2;

static int tsc2007_init(void)
{
	int rc;

	vreg_tsc_s3 = vreg_get(NULL, "s3");
	if (IS_ERR(vreg_tsc_s3)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
		       __func__, PTR_ERR(vreg_tsc_s3));
		return -ENODEV;
	}

	rc = vreg_set_level(vreg_tsc_s3, 1800);
	if (rc) {
		pr_err("%s: vreg_set_level failed \n", __func__);
		goto fail_vreg_set_level;
	}

	rc = vreg_enable(vreg_tsc_s3);
	if (rc) {
		pr_err("%s: vreg_enable failed \n", __func__);
		goto fail_vreg_set_level;
	}

	vreg_tsc_s2 = vreg_get(NULL, "s2");
	if (IS_ERR(vreg_tsc_s2)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
		       __func__, PTR_ERR(vreg_tsc_s2));
		goto fail_vreg_get;
	}

	rc = vreg_set_level(vreg_tsc_s2, 1300);
	if (rc) {
		pr_err("%s: vreg_set_level failed \n", __func__);
		goto fail_vreg_s2_level;
	}

	rc = vreg_enable(vreg_tsc_s2);
	if (rc) {
		pr_err("%s: vreg_enable failed \n", __func__);
		goto fail_vreg_s2_level;
	}

	rc = msm_gpios_request_enable(tsc2007_config_data,
			ARRAY_SIZE(tsc2007_config_data));
	if (rc) {
		pr_err("%s: Unable to request gpios\n", __func__);
		goto fail_gpio_req;
	}

	return 0;

fail_gpio_req:
	vreg_disable(vreg_tsc_s2);
fail_vreg_s2_level:
	vreg_put(vreg_tsc_s2);
fail_vreg_get:
	vreg_disable(vreg_tsc_s3);
fail_vreg_set_level:
	vreg_put(vreg_tsc_s3);
	return rc;
}

static int tsc2007_get_pendown_state(void)
{
	int rc;

	rc = gpio_get_value(TSC2007_TS_PEN_INT);
	if (rc < 0) {
		pr_err("%s: MSM GPIO %d read failed\n", __func__,
						TSC2007_TS_PEN_INT);
		return rc;
	}

	return (rc == 0 ? 1 : 0);
}

static void tsc2007_exit(void)
{
	vreg_disable(vreg_tsc_s3);
	vreg_put(vreg_tsc_s3);
	vreg_disable(vreg_tsc_s2);
	vreg_put(vreg_tsc_s2);

	msm_gpios_disable_free(tsc2007_config_data,
		ARRAY_SIZE(tsc2007_config_data));
}

static int tsc2007_power_shutdown(bool enable)
{
	int rc;

	if (enable == false) {
		rc = vreg_enable(vreg_tsc_s2);
		if (rc) {
			pr_err("%s: vreg_enable failed\n", __func__);
			return rc;
		}
		rc = vreg_enable(vreg_tsc_s3);
		if (rc) {
			pr_err("%s: vreg_enable failed\n", __func__);
			vreg_disable(vreg_tsc_s2);
			return rc;
		}
		/* Voltage settling delay */
		msleep(20);
	} else {
		rc = vreg_disable(vreg_tsc_s2);
		if (rc) {
			pr_err("%s: vreg_disable failed\n", __func__);
			return rc;
		}
		rc = vreg_disable(vreg_tsc_s3);
		if (rc) {
			pr_err("%s: vreg_disable failed\n", __func__);
			vreg_enable(vreg_tsc_s2);
			return rc;
		}
	}

	return rc;
}

static struct tsc2007_platform_data tsc2007_ts_data = {
	.model = 2007,
	.x_plate_ohms = 300,
	.irq_flags    = IRQF_TRIGGER_LOW,
	.init_platform_hw = tsc2007_init,
	.exit_platform_hw = tsc2007_exit,
	.power_shutdown	  = tsc2007_power_shutdown,
	.invert_x	  = true,
	.invert_y	  = true,
	/* REVISIT: Temporary fix for reversed pressure */
	.invert_z1	  = true,
	.invert_z2	  = true,
	.get_pendown_state = tsc2007_get_pendown_state,
};

static struct i2c_board_info tsc_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("tsc2007", 0x48),
		.irq		= MSM_GPIO_TO_INT(TSC2007_TS_PEN_INT),
		.platform_data = &tsc2007_ts_data,
	},
};
#endif

// BEGIN 0010030 : eundeok.bae@lge.com 2010-10-18
// [KERNEL] Removed functions not used in the board-bryce.c

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

// END 0010030 : eundeok.bae@lge.com 2010-10-18

static int kp_flip_mpp_config(void)
{
	return pm8058_mpp_config_digital_in(PM_FLIP_MPP,
		PM8058_MPP_DIG_LEVEL_S3, PM_MPP_DIN_TO_INT);
}

static struct flip_switch_pdata flip_switch_data = {
	.name = "kp_flip_switch",
	.flip_gpio = PM8058_GPIO_PM_TO_SYS(PM8058_GPIOS) + PM_FLIP_MPP,
	.left_key = KEY_OPEN,
	.right_key = KEY_CLOSE,
	.active_low = 0,
	.wakeup = 1,
	.flip_mpp_config = kp_flip_mpp_config,
};

static struct platform_device flip_switch_device = {
	.name   = "kp_flip_switch",
	.id	= -1,
	.dev    = {
		.platform_data = &flip_switch_data,
	}
};


/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	welcome new board.
*/
#if CONFIG_MACH_LGE_BRYCE
static void __init msm7x30_bryce_init(void)
{
	/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
		For power debug. comment out when not in debug mode
	*/
/* ey.cho@lge.com    11.02.27
   START 0017240: [Touch] Changed initialise code that power on after power off  */
	touch_power(0);
/*   END 0017240: [Touch] Changed initialise code that power on after power off  */
#if defined(LG_HW_REV1)
	printk("%s: Welcome hardware board EVB1\n", __func__);
#endif
#if defined(LG_HW_REV2)
	printk("%s: Welcome hardware board EVB2\n", __func__);
#endif
#if defined(LG_HW_REV3)
	printk("%s: Welcome hardware board Rev.A\n", __func__);
#endif
#if defined(LG_HW_REV4)
	printk("%s: Welcome hardware board Rev.B\n", __func__);
#endif

	if (socinfo_init() < 0)
		printk(KERN_ERR "%s: socinfo_init() failed!\n",
		       __func__);
	
	msm_clock_init(msm_clocks_7x30, msm_num_clocks_7x30);

#ifdef CONFIG_SERIAL_MSM_CONSOLE
#ifdef LG_HW_REV1
	msm7x30_init_uart3();
#endif
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	msm7x30_init_uart2();
#endif
#endif

	msm_spm_init(&msm_spm_data, 1);
	msm_acpu_clock_init(&msm7x30_clock_data);

#ifdef CONFIG_USB_FUNCTION
	msm_hsusb_pdata.swfi_latency =
		msm_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	msm_device_hsusb_peripheral.dev.platform_data = &msm_hsusb_pdata;
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
	msm_device_otg.dev.platform_data = &msm_otg_pdata;
#ifdef CONFIG_USB_GADGET
	msm_otg_pdata.swfi_latency =
		msm_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;
#endif
#endif
	msm_uart_dm1_pdata.wakeup_irq = gpio_to_irq(136);
	msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;
/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
	msm_adc_pdata.dev_names = msm_adc_surf_device_names;
	msm_adc_pdata.num_adc = ARRAY_SIZE(msm_adc_surf_device_names);

	platform_add_devices(lge_devices, ARRAY_SIZE(lge_devices));
#ifdef CONFIG_USB_EHCI_MSM
	msm_add_host(0, &msm_usb_host_pdata);
#endif
	msm7x30_init_mmc();
/* sungwoo.cho	10.08.12
	Add misc devices
*/
	lge_add_misc_devices();
	msm7x30_init_nand();
	//msm_qsd_spi_init();
	msm_fb_add_devices();
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	msm_device_i2c_init();
	msm_device_i2c_2_init();
	
	/* chanhee.park	 10.08.06 
		jisun.shin	  10.09.19
	   qup device use */
#if !defined(LG_HW_REV1) && !defined(LG_HW_REV2)		
	qup_device_i2c_init();
#endif
/*	jihye.ahn	10.08.18    
	keyboard-backlight & RGB indicator
*/
/* jihye.ahn@lge.com   10-09-20   add Rev.C board feature */
#if defined(LG_HW_REV4)	|| defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	pmic8058_leds_init();
#endif
	buses_init();
	msm7x30_init_marimba();
#ifdef CONFIG_MSM7KV2_AUDIO
	snddev_poweramp_gpio_init();
	aux_pcm_gpio_init();
#endif

	//bt_power_init();
/* CONFIG_MACH_LGE_BRYCE	hyunjong.do@lge.com 10.08.20
	special clock setting  */
#if CONFIG_MACH_LGE_BRYCE
	pm8058_special_clock0_setting ();
#endif

#if defined(LG_HW_REV1) || defined(LG_HW_REV2)
	/* AMRI5K touchscreen */
	spi_register_board_info(msm_spi_board_info, ARRAY_SIZE(msm_spi_board_info)); 
#endif
	
#ifdef CONFIG_I2C_SSBI
	msm_device_ssbi6.dev.platform_data = &msm_i2c_ssbi6_pdata;
	msm_device_ssbi7.dev.platform_data = &msm_i2c_ssbi7_pdata;
#endif

#if defined (LG_HW_REV3) || defined(LG_HW_REV4)
	init_gpio_i2c_pin(&amp_i2c_pdata, amp_i2c_pin[0], &amp_i2c_bdinfo[0]);
#endif

#if defined(LG_HW_REV2)
	init_gpio_i2c_pin(&tskey_i2c_pdata, tskey_i2c_pin[0], &tskey_i2c_bdinfo[0]);
#endif
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	init_gpio_i2c_pin(&bl_i2c_pdata, bl_i2c_pin[0], &bl_i2c_bdinfo[0]);
	init_gpio_i2c_pin(&sensor_i2c_pdata, sensor_i2c_pin[0], &sensor_i2c_info[1]);
#endif 

	i2c_register_board_info(0, msm_i2c_board_info,
			ARRAY_SIZE(msm_i2c_board_info));

#ifdef CONFIG_TIMPANI_CODEC
	i2c_register_board_info(2, msm_i2c_gsbi7_timpani_info,
			ARRAY_SIZE(msm_i2c_gsbi7_timpani_info));
#endif
	i2c_register_board_info(2, msm_marimba_board_info,
			ARRAY_SIZE(msm_marimba_board_info));

#if defined(LG_HW_REV2)
	i2c_register_board_info(10, tskey_i2c_bdinfo,
			ARRAY_SIZE(tskey_i2c_bdinfo));
#endif

#if defined(LG_HW_REV2) || defined(LG_HW_REV3) ||defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	i2c_register_board_info(12, bl_i2c_bdinfo, 
			ARRAY_SIZE(bl_i2c_bdinfo));
#endif

	bryce_init_i2c_hdmi(13);

#if defined (LG_HW_REV3) || defined(LG_HW_REV4)
	i2c_register_board_info(14, amp_i2c_bdinfo, 
			ARRAY_SIZE(amp_i2c_bdinfo));
#endif

	/* kwangdo.yi S 100706 */
#if defined(LG_HW_REV2) || defined(LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
	i2c_register_board_info(15, sensor_i2c_info,
			ARRAY_SIZE(sensor_i2c_info));
#endif 
// CONFIG_MACH_LGE_BRYCE chanha.park@lge.com    10.08.26
// START : For Bluetooth
#if CONFIG_MACH_LGE_BRYCE
//For Bluetooth Power Init...
	lge_add_btpower_devices();
#endif	
// END : For Bluetooth	

	/* tony.chung	10.07.01 */
	lge_add_camera_devices();

/* CONFIG_MACH_LGE_BRYCE ey.cho@lge.com    10.10.04
   START : For volume key */
#if defined(LG_HW_REV3) || defined(LG_HW_REV4)
	gpio_tlmm_config(GPIO_CFG(19, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(20, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(56, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#else
	gpio_tlmm_config(GPIO_CFG(20, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(19, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(56, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif

/* END : For volume key*/
}
#endif

static unsigned pmem_sf_size = MSM_PMEM_SF_SIZE;
static int __init pmem_sf_size_setup(char **p)
{
	pmem_sf_size = memparse(*p, NULL);
	return 0;
}
early_param("pmem_sf_size=", pmem_sf_size_setup);

static unsigned fb_size = MSM_FB_SIZE;
static int __init fb_size_setup(char **p)
{
	fb_size = memparse(*p, NULL);
	return 0;
}
early_param("fb_size=", fb_size_setup);

/*static unsigned gpu_phys_size = MSM_GPU_PHYS_SIZE;
static int __init gpu_phys_size_setup(char **p)
{
	gpu_phys_size = memparse(*p, NULL);
	return 0;
}
early_param("gpu_phys_size=", gpu_phys_size_setup);*/ //lavanya commented

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char **p)
{
	pmem_adsp_size = memparse(*p, NULL);
	return 0;
}
early_param("pmem_adsp_size=", pmem_adsp_size_setup);

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;
static int __init pmem_audio_size_setup(char **p)
{
	pmem_audio_size = memparse(*p, NULL);
	return 0;
}
early_param("pmem_audio_size=", pmem_audio_size_setup);

static unsigned pmem_kernel_ebi1_size = PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char **p)
{
	pmem_kernel_ebi1_size = memparse(*p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size=", pmem_kernel_ebi1_size_setup);

static void __init msm7x30_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;
/*
   Request allocation of Hardware accessible PMEM regions
   at the beginning to make sure they are allocated in EBI-0.
   This will allow 7x30 with two mem banks enter the second
   mem bank into Self-Refresh State during Idle Power Collapse.

    The current HW accessible PMEM regions are
    1. Frame Buffer.
       LCDC HW can access msm_fb_resources during Idle-PC.

    2. Audio
       LPA HW can access android_pmem_audio_pdata during Idle-PC.
*/
	size = fb_size ? : MSM_FB_SIZE;
	addr = alloc_bootmem(size);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
		size, addr, __pa(addr));

	size = pmem_audio_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_audio_pdata.start = __pa(addr);
		android_pmem_audio_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for audio "
			"pmem arena\n", size, addr, __pa(addr));
	}

/*	size = gpu_phys_size;
	if (size) {
		addr = alloc_bootmem(size);
		kgsl_resources[1].start = __pa(addr);
		kgsl_resources[1].end = kgsl_resources[1].start + size - 1;
		pr_info("allocating %lu bytes at %p (%lx physical) for "
			"KGSL\n", size, addr, __pa(addr));
	}
*/ //lavanya commented
	size = pmem_kernel_ebi1_size;
	if (size) {
		addr = alloc_bootmem_aligned(size, 0x100000);
		android_pmem_kernel_ebi1_pdata.start = __pa(addr);
		android_pmem_kernel_ebi1_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for kernel"
			" ebi1 pmem arena\n", size, addr, __pa(addr));
	}

	size = pmem_sf_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_pdata.start = __pa(addr);
		android_pmem_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for sf "
			"pmem arena\n", size, addr, __pa(addr));
	}

/* neo.kang@lge.com	10.12.15.
 * 0012867 : delete the code related with fluid */
	size = pmem_adsp_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_adsp_pdata.start = __pa(addr);
		android_pmem_adsp_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for adsp "
			"pmem arena\n", size, addr, __pa(addr));
	}
}

static void __init msm7x30_map_io(void)
{
	msm_shared_ram_phys = 0x00100000;
	msm_map_msm7x30_io();
	msm7x30_allocate_memory_regions();
}

/* CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.07.10
	welcome new board.
*/
#if CONFIG_MACH_LGE_BRYCE
MACHINE_START(LGE_BRYCE, "LGE BRYCE BOARD")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io  = MSM_DEBUG_UART_PHYS,
	.io_pg_offst = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params = PHYS_OFFSET + 0x100,
	.map_io = msm7x30_map_io,
	.init_irq = msm7x30_init_irq,
	.init_machine = msm7x30_bryce_init,
	.timer = &msm_timer,
MACHINE_END
#endif

