/* LGE_CHANGE_S [joribbong77@lge.com] 2009-12-13, migration of EVE error handle feature */
/****************************************************************************
 * Created by [bluerti@lge.com]
 * 2009-07-06
 * Made this file for implementing LGE Error Hanlder 
 * *************************************************************************/
#include "lge_errorhandler.h"

#include <mach/msm_smd.h>
#include <mach/msm_iomap.h>
#include <mach/system.h>
#include <linux/io.h>
#include <linux/syscalls.h>

#include "../smd_private.h"
#include "../proc_comm.h"
#include "../modem_notifier.h"

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>

int LG_ErrorHandler_enable = 0;
static int user_keypress =0;

int LGE_ErrorHandler_Main( int crash_side, char * message)
{
	char * kmem_buf;
	LG_ErrorHandler_enable = 1;
	raw_local_irq_enable();
	/* kwangdo.yi@lge.com 10.11.04  S 
	 * 0010515: temporary disable CONFIG_FRAMEBUFFER_CONSOLE because of current consumption
	*/
#if 0 
	kmem_buf = kmalloc(LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN, GFP_ATOMIC);
	memcpy(kmem_buf, message, LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN);
	switch(crash_side) {
		case MODEM_CRASH:
			display_info_LCD(crash_side, message);
			break;

		case APPL_CRASH:
			display_info_LCD(crash_side, message);
			break;


	}
	kfree(kmem_buf);
#endif
	/* kwangdo.yi@lge.com 10.11.04 E */

	raw_local_irq_disable();
	preempt_disable();
	/* BEGIN: 0006765 dongwook.lee@lge.com 2010-06-03 */
/* MODIFY 0006765: Fix : The kernel panic screen is disappeared just after linux kernel gets system panic */
	mdelay(100);
 
	while(1)
	{
		// 1. Check Volume Key 
/* kwangdo.yi@lge.com 10.11.03 S 
0010460: add HW revision feature to vol key gpio in kernel panic
*/
#if CONFIG_MACH_LGE_BRYCE
#if defined (LG_HW_REV2)
		gpio_direction_output(19,1);
		gpio_direction_output(56,0);	//volume down
		gpio_direction_output(20,1);	//volume up
		gpio_direction_input(19);
		if(gpio_get_value(19)==0){
			printk("### vol down key pressed \n");
			mdelay(100);
/* kwangdo.yi@lge.com 10.11.03 S 
   0010472: each volup/voldown key mapped separately to reboot/download mode. 
 */
			return SMSM_SYSTEM_DOWNLOAD; 
/* kwangdo.yi@lge.com 10.11.03 E */
		}
		mdelay(100);
		gpio_direction_output(19,1);
		gpio_direction_output(20,0);
		gpio_direction_output(56,1);
		gpio_direction_input(19);
		if(gpio_get_value(19)==0){
			printk("### vol up key pressed\n");
			mdelay(100);
			return SMSM_SYSTEM_REBOOT;
		
		}
		mdelay(100);
#endif
#if defined (LG_HW_REV3) || defined(LG_HW_REV4) || defined(LG_HW_REV5) || defined(LG_HW_REV6) || defined(LG_HW_REV7)
		gpio_direction_output(20,1);
		gpio_direction_output(56,0);	//volume down
		gpio_direction_output(19,1);	//volume up
		gpio_direction_input(20);
		if(gpio_get_value(20)==0){
			printk("### vol down key pressed \n");
			mdelay(100);
/* kwangdo.yi@lge.com 10.11.03 S 
   0010472: each volup/voldown key mapped separately to reboot/download mode. 
 */
			return SMSM_SYSTEM_DOWNLOAD;
/* kwangdo.yi@lge.com 10.11.03 E */
		}
		mdelay(100);
		gpio_direction_output(20,1);
		gpio_direction_output(19,0);
		gpio_direction_output(56,1);
		gpio_direction_input(20);
		if(gpio_get_value(20)==0){
			printk("### vol up key pressed\n");
			mdelay(100);
			return SMSM_SYSTEM_REBOOT; 
		
		}
		mdelay(100);
#endif
#endif
/* kwangdo.yi@lge.com 10.11.03 E */ 
	}
/* END: 0006765 dongwook.lee@lge.com 2010-06-03 */

	
 }

	/* kwangdo.yi@lge.com 10.11.04 S 
	 * 0010515: temporary disable CONFIG_FRAMEBUFFER_CONSOLE because of current consumption
	*/
#if 0 
int display_info_LCD( int crash_side, char * message)
{
	unsigned short * buffer;
	
	buffer = kmalloc(LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN*sizeof(short), GFP_ATOMIC);


	if(buffer)
		expand_char_to_shrt(message,buffer);
	else
		printk("Memory Alloc failed!!\n");

	display_errorinfo_byLGE(crash_side, buffer,LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN );

	kfree(buffer);

	return 0;
}
#endif

void expand_char_to_shrt(char * message,unsigned short *buffer)
{
	char * src = message;
	unsigned char  * dst = (unsigned char *)buffer;
	int i=0;


	for(i=0;i<LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN; i++) {
		*dst++ = *src++;
		*dst++ = 0x0;
	}
	
}

/* LGE_CHANGE_E [joribbong77@lge.com] 2009-12-30 */
