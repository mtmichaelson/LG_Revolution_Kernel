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

#ifndef __ASM__ARCH_MSM_LG_PCB_VERSION_H
#define __ASM__ARCH_MSM_LG_PCB_VERSION_H

/* BEGIN: 0015325 jihoon.lee@lge.com 20110204 */
/* ADD 0015325: [KERNEL] HW VERSION FILE */
// this should be sync up with CP
typedef enum {
	PCB_REVISION_A = 14,
	PCB_REVISION_B = 27,
	PCB_REVISION_C = 41,
	PCB_REVISION_D = 54,
	PCB_REVISION_E = 68,
	PCB_REVISION_F = 82,
	PCB_REVISION_G = 92,
	PCB_REVISION_H = 109,
	PCB_REVISION_I = 128,
	PCB_REVISION_1P0 = 144,
	PCB_REVISION_1P1 = 164,
	PCB_REVISION_1P2 = 180,
	PCB_REVISION_UNKOWN = 0xFF
}hw_pcb_version_type;

int lg_get_board_pcb_version(void);
void lg_set_hw_version_string(char *pcb_version, int size);
/* END: 0015325 jihoon.lee@lge.com 20110204 */
#endif
