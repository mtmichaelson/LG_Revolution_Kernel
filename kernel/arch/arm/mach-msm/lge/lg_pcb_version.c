/* arch/arm/mach-msm/include/mach/lg_pcb_version.h
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
#include <linux/string.h>
#include <linux/kernel.h>

#include <mach/lg_pcb_version.h>
#include <mach/lg_diag_testmode.h>

/* BEGIN: 0015325 jihoon.lee@lge.com 20110204 */
/* ADD 0015325: [KERNEL] HW VERSION FILE */
#ifdef CONFIG_LGE_PCB_VERSION
extern byte CheckHWRev(void);

typedef struct {
	hw_pcb_version_type adc;
	char *pStr;
} PcbRev;

static PcbRev pPcbRev[] = {
	{PCB_REVISION_A, "Rev.A\n"}, 
	{PCB_REVISION_B, "Rev.B\n"},
	{PCB_REVISION_C, "Rev.C\n"}, 
	{PCB_REVISION_D, "Rev.D\n"},
	{PCB_REVISION_E, "Rev.E\n"}, 
	{PCB_REVISION_F, "Rev.F\n"}, 
	{PCB_REVISION_G, "Rev.G\n"}, 
	{PCB_REVISION_H, "Rev.H\n"}, 
	{PCB_REVISION_I, "Rev.I\n"}, 
	{PCB_REVISION_1P0, "Rev.1.0\n"}, 
	{PCB_REVISION_1P1, "Rev.1.1\n"}, 
	{PCB_REVISION_1P2, "Rev.1.2\n"}, 
	{PCB_REVISION_UNKOWN, "Unknown\n"},
};

int lg_get_board_pcb_version(void)
{
    static byte pcb_version = PCB_REVISION_UNKOWN;
    
    if(pcb_version == PCB_REVISION_UNKOWN)
    {
        pcb_version = CheckHWRev();
    }

    return pcb_version;
}

void lg_set_hw_version_string(char *pcb_version, int size)
{
	hw_pcb_version_type hw_version;
	int i = 0;

	hw_version=lg_get_board_pcb_version();
	
	for(i = 0; i < sizeof(pPcbRev) / sizeof(PcbRev) - 1; i++)
	{
		if(hw_version <= pPcbRev[i].adc)
			break;
	}

	memset(pcb_version, 0, size);

	if(pPcbRev[i].adc < PCB_REVISION_1P0)
		memcpy((void *)pcb_version, (const void *)pPcbRev[i].pStr, 6);
	else
		memcpy((void *)pcb_version, (const void *)pPcbRev[i].pStr, 8);

	printk(KERN_INFO "%s, adc : %d, str : %s\n",__func__, pPcbRev[i].adc, pPcbRev[i].pStr);
}

#endif /* CONFIG_LGE_PCB_VERSION */
/* END: 0015325 jihoon.lee@lge.com 20110204 */

