/*
 * LGE L2000 HIM layer
 * 
 * Jae-gyu Lee <jaegyu.lee@lge.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
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
*/

#include "lte_sdio.h"
#include "lte_debug.h"
#include "lte_him.h"
#include <linux/slab.h> 
/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */
#include <linux/gpio.h>
#include <linux/unistd.h>
/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   			

extern PLTE_SDIO_INFO gLte_sdio_info;

/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */


#ifdef SDCC_STATUS_MONITORING
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
extern void msmsdcc_print_sdcc_status();
#endif

#ifdef LTE_CRASH_DUMP
#include <linux/syscalls.h>
#include <asm/uaccess.h>
int lte_panic_report_prev(unsigned char* lte_log_buf_prev, unsigned int size);
int lte_panic_report_after(unsigned char* lte_log_buf_after, unsigned int size);
#endif /* LTE_CRASH_DUMP */
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
/* BEGIN: 0013575 seongmook.yim@lge.com 2011-01-05 */
/* ADD 0013575: [LTE] LTE S/W assert case handling */
extern int lte_crash_log(void *buffer, unsigned int size, unsigned int reserved);
/* END: 0013575 seongmook.yim@lge.com 2011-01-05 */   

static unsigned int lte_him_calc_dummy(unsigned int packet_size)
{
	unsigned int temp_align, dummy_size, total_packet_size = 0;
	
	temp_align = packet_size % HIM_BLOCK_BYTE_ALIGN;
	
	if(temp_align)	
		dummy_size = HIM_BLOCK_BYTE_ALIGN - temp_align;
/* defect */
/* when temp_align is false(0) dummy_size should defined */
	else
		dummy_size = 0;
/* defect */	
	total_packet_size = packet_size + dummy_size ;

	return	total_packet_size;
}

int lte_him_register_cb_from_ved(lte_him_cb_fn_t cb)
{
	if (gLte_sdio_info == NULL) {
		return -ENXIO;
	}
	
	gLte_sdio_info->callback_from_ved = cb;

	return 0;
}

int lte_him_enqueue_ip_packet(unsigned char *data, int size, int pdn)
{
	int error = 0;
	unsigned int packet_length, curr_pdn = 0;
	unsigned long flags;
	unsigned char *tmp_packet;	
	tx_packet_list *curr_item;


/* BEGIN: 0018522 jaegyu.lee@lge.com 2011-03-24 */
/* MOD 0018522: [LTE] Kernel crash happen, When the SDIO TX & RX fail in 5sec */
	if(gLte_sdio_info->flag_gpio_l2k_host_status == FALSE){
		/* BEGIN: 0018684 seungyeol.seo@lge.com 2011-03-30 */
		/* MOD 0018684: [LTE] Remove a debug code for tracking the unwoken host */
		// LTE_INFO(" *********** MSM SDCC doesn't resume *********** \n");
		/* END: 0018684 seungyeol.seo@lge.com 2011-03-30 */
		return -EIO;
	}
/* END: 0018522 jaegyu.lee@lge.com 2011-03-24 */

	if (gLte_sdio_info == NULL) {
		return -ENXIO;
	}
	if(size==0 || data == NULL)
	{
		return -EINVAL;
	}

/*BEGIN: 0017497 daeok.kim@lge.com 2011-03-05 */
/*MOD 0017497: [LTE] LTE SW Assert stability is update: blocking of tx operation */
	if (gLte_sdio_info->flag_tx_blocking == TRUE)
	{
		LTE_ERROR("[LTE_ASSERT] SDIO Tx(IP data) blocking, return -EIO \n");
		return -EIO;
	}
/*END: 0017497 daeok.kim@lge.com 2011-03-05 */

	mutex_lock(&gLte_sdio_info->tx_lock_mutex);	

	/* packet_length = 4 byte aligned HIM packet size */
	packet_length = lte_him_calc_dummy(HIM_PACKET_HEADER_SIZE + size);

	/* memory allocation for tx packet list */
	curr_item = (tx_packet_list *)kzalloc(sizeof(tx_packet_list), GFP_KERNEL);
	if(!curr_item)
	{
/* BEGIN: 0011698 jaegyu.lee@lge.com 2010-12-01 */
/* ADD 0011698: [LTE] ADD : Debug message for him packet */
		LTE_ERROR("Failed to allocate memory\n");
/* END : 0011698 jaegyu.lee@lge.com 2010-12-01 */
		mutex_unlock(&gLte_sdio_info->tx_lock_mutex);			
		return -ENOMEM;
	}

	/* memory allocation for tx packet */
	tmp_packet = (unsigned char *)kzalloc(packet_length, GFP_KERNEL);
	if(!tmp_packet)
	{
/* BEGIN: 0011698 jaegyu.lee@lge.com 2010-12-01 */
/* ADD 0011698: [LTE] ADD : Debug message for him packet */

		LTE_ERROR("Failed to allocate memory\n");
/* END : 0011698 jaegyu.lee@lge.com 2010-12-01 */
		kfree(curr_item);
		mutex_unlock(&gLte_sdio_info->tx_lock_mutex);			
		return -ENOMEM;
	}

	curr_item->tx_packet = tmp_packet;
	curr_item->size = packet_length;	

	switch(pdn)
	{
		case 0:
			curr_pdn = HIM_NIC_1;
			break;
		case 1:
			curr_pdn = HIM_NIC_2;			
			break;
		case 2:
			curr_pdn = HIM_NIC_3;			
			break;
		case 3:
			curr_pdn = HIM_IP_PACKET_1;
			break;
		case 4:
			curr_pdn = HIM_IP_PACKET_2;			
			break;
		case 5:
			curr_pdn = HIM_IP_PACKET_3;
			break;
		case 6:
			curr_pdn = HIM_IP_PACKET_4;			
			break;
		case 7:
			curr_pdn = HIM_IP_PACKET_5;	
			break;
		case 8:
			curr_pdn = HIM_IP_PACKET_6;			
			break;
		case 9:
			curr_pdn = HIM_IP_PACKET_7;	
			break;
		case 10:
			curr_pdn = HIM_IP_PACKET_8;	
			break;
		default:
/* defect */
/* When default case, free the memory */
			mutex_unlock(&gLte_sdio_info->tx_lock_mutex);				
			kfree(curr_item);
/* defect */

/* BEGIN: 0011698 jaegyu.lee@lge.com 2010-12-01 */
/* ADD 0011698: [LTE] ADD : Debug message for him packet */
			LTE_ERROR("Unsupported PDN\n");
/* END : 0011698 jaegyu.lee@lge.com 2010-12-01 */
			return -EIO;
	}


	((him_packet_header *)(tmp_packet))->payload_size = (unsigned short)size;	
	((him_packet_header *)(tmp_packet))->packet_type = curr_pdn;

	memcpy(tmp_packet + HIM_PACKET_HEADER_SIZE, data, size);

	list_add_tail(&curr_item->list, &gLte_sdio_info->tx_packet_head->list);
	gLte_sdio_info->tx_list_size += packet_length;

	mutex_unlock(&gLte_sdio_info->tx_lock_mutex);	

#ifndef SDIO_TX_TIMER
	queue_work(gLte_sdio_info->tx_workqueue, &gLte_sdio_info->tx_worker);
#endif

	return error;
}


int lte_him_write_control_packet(const unsigned char *data, int size)
{
	unsigned char *tmp_him_blk;
	int error, blk_count = 0;
	unsigned int him_blk_length, packet_length, write_length = 0;
	unsigned long flags;

/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
#ifdef SDCC_STATUS_MONITORING
	/* To unpack a RIL packet */
	unsigned char	*current_buffer = data;
	lte_sdio_hi_command_header *header;
	header = (lte_sdio_hi_command_header *)current_buffer;
	
	gLte_sdcc_status.tracking[gLte_sdcc_status.index] = enum_lte_him_write_control_packet;
	if(gLte_sdcc_status.index == 29) gLte_sdcc_status.index = 0;
	else gLte_sdcc_status.index++;
#endif /* SDCC_STATUS_MONITORING */

/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
	if (gLte_sdio_info == NULL) {
		return -ENXIO;
	}


//	FUNC_ENTER();
	/* BEGIN: 0018385 seungyeol.seo@lge.com 2011-03-22 */
	/* MOD 0018385 : [LTE] Rearrangement of 'Wake Lock' section */
	//mutex_lock(&gLte_sdio_info->tx_lock_mutex);
	/* END: 0018385 seungyeol.seo@lge.com 2011-03-22 */

/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */	
#ifdef LTE_WAKE_UP
	gpio_set_value(GPIO_L2K_LTE_WAKEUP,TRUE);			//SET LTE_WU
#endif	
/* BEGIN: 0015024 jaegyu@lge.com 2011-01-29 */
/* MOD 0015024: [LTE] AP power collapse move wake lock point */
	/* Wake lock for 0.5 sec */
/* END: 0015024 jaegyu@lge.com 2011-01-29 */
/* BEGIN: 0017997 jaegyu.lee@lge.com 2011-03-15 */
/* MOD 0017997: [LTE] Wake-lock period rearrange in LTE SDIO driver */
//	wake_lock_timeout(&gLte_sdio_info->lte_wake_lock, HZ / 2);
/* END: 0017997 jaegyu.lee@lge.com 2011-03-15 */   

/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   			

	packet_length = lte_him_calc_dummy(HIM_PACKET_HEADER_SIZE + size);
	him_blk_length = HIM_BLOCK_HEADER_SIZE + packet_length;
	
	tmp_him_blk = (unsigned char *)kzalloc(him_blk_length, GFP_KERNEL);
	if(!tmp_him_blk)
	{

/* BEGIN: 0011698 jaegyu.lee@lge.com 2010-12-01 */
/* ADD 0011698: [LTE] ADD : Debug message for him packet */
		LTE_ERROR("Failed to allocate memory\n");
/* END : 0011698 jaegyu.lee@lge.com 2010-12-01 */

/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */	
#ifdef LTE_WAKE_UP
	        gpio_set_value(GPIO_L2K_LTE_WAKEUP,FALSE);	//SET LTE_WU
#endif			    
/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   			

		/* BEGIN: 0018385 seungyeol.seo@lge.com 2011-03-22 */
		/* MOD 0018385 : [LTE] Rearrangement of 'Wake Lock' section */
		//mutex_unlock(&gLte_sdio_info->tx_lock_mutex);			
		/* END: 0018385 seungyeol.seo@lge.com 2011-03-22 */
		return -ENOMEM;
	}
	
	((him_block_header *)(tmp_him_blk))->total_block_size = him_blk_length;
	((him_block_header *)(tmp_him_blk))->seq_num = gLte_sdio_info->him_tx_cnt;	
	((him_block_header *)(tmp_him_blk))->packet_num = 1;
/* BEGIN: 0012603 jaegyu.lee@lge.com 2010-12-17 */
/* MOD 0012603: [LTE] HIM block parsing policy change */
	((him_block_header *)(tmp_him_blk))->him_block_type = HIM_BLOCK_MISC;
/* END: 0012603 jaegyu.lee@lge.com 2010-12-17 */

	((him_packet_header *)(tmp_him_blk + HIM_BLOCK_HEADER_SIZE))->payload_size = (unsigned short)size;	
	((him_packet_header *)(tmp_him_blk + HIM_BLOCK_HEADER_SIZE))->packet_type = HIM_CONTROL_PACKET;	

	memcpy(tmp_him_blk + HIM_BLOCK_HEADER_SIZE + HIM_PACKET_HEADER_SIZE, data, size);

	if((him_blk_length % LTE_SDIO_BLK_SIZE) != 0)
	{
		blk_count = (him_blk_length / LTE_SDIO_BLK_SIZE);
		write_length = (LTE_SDIO_BLK_SIZE * blk_count) + LTE_SDIO_BLK_SIZE;
	}
	else
	{
		write_length = him_blk_length;
	}

#ifndef LGE_NO_HARDWARE

/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */	
#ifdef LTE_WAKE_UP
	//printk(" ************* lte_him_write_control_packet 1**************\n");		
/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
#ifdef SDCC_STATUS_MONITORING
		gLte_sdcc_status.tracking[gLte_sdcc_status.index] = enum_lte_sdio_wake_up_tx;
		if(gLte_sdcc_status.index == 29) gLte_sdcc_status.index = 0;
		else gLte_sdcc_status.index++;
	printk("[CmdID:0x%04x, Length:%d]",header->cmd, size);
#endif /* SDCC_STATUS_MONITORING */
	error = lte_sdio_wake_up_tx((void *)tmp_him_blk, write_length);					
#ifdef SDCC_STATUS_MONITORING
	msmsdcc_print_sdcc_status();
#endif /* SDCC_STATUS_MONITORING */				
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
#else
	sdio_claim_host(gLte_sdio_info->func);
	error = sdio_memcpy_toio(gLte_sdio_info->func, DATA_PORT, (void*)tmp_him_blk, write_length);			
	sdio_release_host(gLte_sdio_info->func);
#endif
/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   			

#else
	error = 0;
#endif

	if(error != 0)
	{
		LTE_ERROR("Failed to wrtie data to LTE. Error = 0x%x\n", error);
	}
	else
	{
		if(gLte_sdio_info->him_tx_cnt == 0xFFFFFFFF)
			gLte_sdio_info->him_tx_cnt = 1;
		else
			gLte_sdio_info->him_tx_cnt++;
	}

	kfree(tmp_him_blk);

/* BEGIN: 0013584: jihyun.park@lge.com 2011-01-05  */
/* [LTE] Wakeup Scheme  for Power Saving Mode   */	
#ifdef LTE_WAKE_UP
	gpio_set_value(GPIO_L2K_LTE_WAKEUP,FALSE);	//SET LTE_WU
#endif			    
/* END: 0013584: jihyun.park@lge.com 2011-01-05 */   			

	/* BEGIN: 0018385 seungyeol.seo@lge.com 2011-03-22 */
	/* MOD 0018385 : [LTE] Rearrangement of 'Wake Lock' section */
	//mutex_unlock(&gLte_sdio_info->tx_lock_mutex);
	/* END: 0018385 seungyeol.seo@lge.com 2011-03-22 */

//	FUNC_EXIT();

	return error;
}

int lte_him_parsing_blk(void)
{
	unsigned char *rx_buff, *packet_buff;
	unsigned int him_blk_size = 0, him_blk_curr_seq_num = 0, him_blk_seq_num = 0, packet_count = 0, total_packet_size = 0, i = 0;
/* BEGIN: 0012603 jaegyu.lee@lge.com 2010-12-17 */
/* MOD 0012603: [LTE] HIM block parsing policy change */
	unsigned int him_blk_type = 0, real_packet_size = 0, him_packet_type = 0, curr_pdn = 0;
/* END: 0012603 jaegyu.lee@lge.com 2010-12-17 */
	int error;
/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
#ifdef SDCC_STATUS_MONITORING
	/* Unpack a packet to identify cmd ID and length */
	lte_sdio_hi_command_header *header;
#endif /* SDCC_STATUS_MONITORING */
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
	
	rx_buff = gLte_sdio_info->rx_buff;
	him_blk_size = ((him_block_header *)rx_buff)->total_block_size;
	him_blk_curr_seq_num = ((him_block_header *)rx_buff)->seq_num;
	him_blk_seq_num = gLte_sdio_info->him_rx_cnt;
	packet_count = ((him_block_header *)rx_buff)->packet_num;
	him_blk_type = ((him_block_header *)rx_buff)->him_block_type;


/* BEGIN: 0011323 jaegyu.lee@lge.com 2010-11-24 */  
/* MOD 0011323: [LTE] VED callback value = 0 Issue  */  
#if 0
	if(him_blk_curr_seq_num != him_blk_seq_num)
	{
		error = -1;
		LTE_ERROR("HIM rx seq num is wrong! current = %d, received = %d\n", him_blk_seq_num, him_blk_curr_seq_num);
		goto err;
	}
#endif
/* END: 0011213 jaegyu.lee@lge.com 2010-11-24 */ 

/* BEGIN: 0012603 jaegyu.lee@lge.com 2010-12-17 */
/* MOD 0012603: [LTE] HIM block parsing policy change */
	if(him_blk_type == HIM_BLOCK_MISC)
	{	/* Control and ETC. */
		rx_buff += HIM_BLOCK_HEADER_SIZE;

		for(i=0; i < packet_count; i++)
		{
			real_packet_size = ((him_packet_header *)rx_buff)->payload_size;
			him_packet_type = ((him_packet_header *)rx_buff)->packet_type;

			LTE_INFO("MISC packet type = 0x%x\n", him_packet_type);
			LTE_INFO("MISC packet size = %d\n", real_packet_size);

/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
			#ifdef SDCC_STATUS_MONITORING
			header = (lte_sdio_hi_command_header *)(rx_buff + HIM_PACKET_HEADER_SIZE);
			//LTE_INFO("MISC packet type = 0x%x\n", him_packet_type);
			LTE_INFO("MISC packet received (CmdID:0x%04x, Length=%d)\n", header->cmd, real_packet_size);
			#endif /* SDCC_STATUS_MONITORING */
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */

			if(him_packet_type == HIM_CONTROL_PACKET)
			{
				/* Fill the flip buffer */
				tty_insert_flip_string(gLte_sdio_info->tty, rx_buff + HIM_PACKET_HEADER_SIZE, (size_t)real_packet_size);
			}
			else
			{
				LTE_INFO("Not a control packet!\n");
			}

			total_packet_size = lte_him_calc_dummy(HIM_PACKET_HEADER_SIZE + ((him_packet_header *)rx_buff)->payload_size);
			rx_buff += total_packet_size;
		}

		/* wake up tty core */
		tty_flip_buffer_push(gLte_sdio_info->tty);		
	}

	else if(him_blk_type == HIM_BLOCK_IP)
	{	/* IP packet to VED */
		// Parsing HIM block and call VED CB fucntion

		rx_buff += HIM_BLOCK_HEADER_SIZE;
		
/* BEGIN: 0011323 jaegyu.lee@lge.com 2010-11-24 */  
/* MOD 0011323: [LTE] VED callback value = 0 Issue  */  
#if 0
		if(gLte_sdio_info->callback_from_ved == 0)
		{
			error = -2;
			LTE_ERROR("Callback from VED is NULL!\n");
			goto err;
		}
#endif
/* END: 0011213 jaegyu.lee@lge.com 2010-11-24 */ 		

/* END: 0012603 jaegyu.lee@lge.com 2010-12-17 */
		for(i=0; i < packet_count; i++)
		{
			/* Copy IP packet from HIM blk to each packet buffer */
			real_packet_size = ((him_packet_header *)rx_buff)->payload_size;
			packet_buff = (unsigned char *)kzalloc(real_packet_size, GFP_KERNEL);
			if(!packet_buff)
			{
/* BEGIN: 0011698 jaegyu.lee@lge.com 2010-12-01 */
/* ADD 0011698: [LTE] ADD : Debug message for him packet */
				LTE_ERROR("Failed to allocate memory\n");
/* END : 0011698 jaegyu.lee@lge.com 2010-12-01 */
				return -ENOMEM;
			}
			
			memcpy(packet_buff, rx_buff + HIM_PACKET_HEADER_SIZE, real_packet_size);			

			switch(((him_packet_header *)rx_buff)->packet_type)
			{
				case HIM_NIC_1:
					curr_pdn = 0;
					break;
				case HIM_NIC_2:
					curr_pdn = 1;			
					break;
				case HIM_NIC_3:
					curr_pdn = 2;			
					break;
				case HIM_IP_PACKET_1:
					curr_pdn = 3;
					break;
				case HIM_IP_PACKET_2:
					curr_pdn = 4;			
					break;
				case HIM_IP_PACKET_3:
					curr_pdn = 5;			
					break;
				case HIM_IP_PACKET_4:
					curr_pdn = 6;
					break;
				case HIM_IP_PACKET_5:
					curr_pdn = 7;			
					break;
				case HIM_IP_PACKET_6:
					curr_pdn = 8;			
					break;
				case HIM_IP_PACKET_7:
					curr_pdn = 9;			
					break;
				case HIM_IP_PACKET_8:
					curr_pdn = 10;			
					break;
				default:
/* defect */
/* When default case, free the memory */
					kfree(packet_buff);					
/* defect */
					return -1;
			}

//			LTE_INFO("Size = %d\n", real_packet_size);
//			LTE_INFO("PDN = %d\n", curr_pdn);

/* BEGIN: 0011323 jaegyu.lee@lge.com 2010-11-24 */  
/* MOD 0011323: [LTE] VED callback value = 0 Issue  */  
			if(gLte_sdio_info->callback_from_ved != 0)
			{
				(gLte_sdio_info->callback_from_ved)(packet_buff, real_packet_size, curr_pdn);			
			}
/* END: 0011213 jaegyu.lee@lge.com 2010-11-24 */ 

/* BEGIN: 0011698 jaegyu.lee@lge.com 2010-12-01 */
/* ADD 0011698: [LTE] ADD : Debug message for him packet */
			else
			{
				gLte_sdio_info->him_dropped_ip_cnt ++;
				LTE_INFO("Dropped IP packet cnt = %d\n", gLte_sdio_info->him_dropped_ip_cnt);
			}
/* END : 0011698 jaegyu.lee@lge.com 2010-12-01 */
			kfree(packet_buff);
			
			total_packet_size = lte_him_calc_dummy(HIM_PACKET_HEADER_SIZE + ((him_packet_header *)rx_buff)->payload_size);
			rx_buff += total_packet_size;			
		}
		
	}
/* BEGIN: 0012603 jaegyu.lee@lge.com 2010-12-17 */
/* ADD 0012603: [LTE] HIM block parsing policy change */

/* BEGIN: 0013575 seongmook.yim@lge.com 2011-01-05 */
/* ADD 0013575: [LTE] LTE S/W assert case handling */
	else if(him_blk_type == HIM_BLOCK_ERROR)
	{
		/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
		/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
		#ifdef LTE_CRASH_DUMP
		if(gLte_sdio_info->lte_crash_dump_flag == TRUE)
		{
			printk("[LTE_ASSERT] GPIO_L2K_STATUS = High \n");
			
			gLte_sdio_info->flag_tx_blocking = TRUE;
			rx_buff += HIM_BLOCK_HEADER_SIZE;

			for(i=0; i < packet_count; i++)
			{
				real_packet_size = ((him_packet_header *)rx_buff)->payload_size;
				packet_buff = (unsigned char *)kzalloc(real_packet_size, GFP_KERNEL);
					if(!packet_buff)
					{
						LTE_ERROR("Failed to allocate for Error Manager\n");
						return -ENOMEM;
					}
				memcpy(packet_buff,rx_buff + HIM_PACKET_HEADER_SIZE,real_packet_size);
			
		#ifdef SDCC_STATUS_MONITORING
				msmsdcc_print_sdcc_status();
		#endif /* SDCC_STATUS_MONITORING */			

				printk("[LTE_ASSERT] lte_ers_panic_after dump start\n");
				lte_panic_report_after(packet_buff,real_packet_size);
				
				gLte_sdio_info->error_packet = TRUE;
				printk("[LTE_ASSERT] error_packet = %d\n",gLte_sdio_info->error_packet);
				
				kfree(packet_buff);
			}
		}
		else
		{
	#endif /* LTE_CRASH_DUMP */
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
		/* LTE error handling */
		printk("[LTE_ASSERT] LTE S/W Assert Case\n");
/*BEGIN: 0017497 daeok.kim@lge.com 2011-03-05 */
/*MOD 0017497: [LTE] LTE SW Assert stability is update: blocking of tx operation */
		/* SDIO Tx Blocking procedure*/
#if 0 //not available
		if (gLte_sdio_info->tx_workqueue != NULL)
		{
//			flush_workqueue(gLte_sdio_info->tx_workqueue);
			destroy_workqueue(gLte_sdio_info->tx_workqueue);
			gLte_sdio_info->tx_workqueue = NULL;
		}
#endif
		gLte_sdio_info->flag_tx_blocking =TRUE;
/*END: 0017497 daeok.kim@lge.com 2011-03-05 */		
		rx_buff += HIM_BLOCK_HEADER_SIZE;

		for(i=0; i < packet_count; i++)
		{
			real_packet_size = ((him_packet_header *)rx_buff)->payload_size;
			packet_buff = (unsigned char *)kzalloc(real_packet_size, GFP_KERNEL);
				if(!packet_buff)
				{
					LTE_ERROR("Failed to allocate for Error Manager\n");
					return -ENOMEM;
				}
			memcpy(packet_buff,rx_buff + HIM_PACKET_HEADER_SIZE,real_packet_size);
			
/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
		#ifdef SDCC_STATUS_MONITORING
				msmsdcc_print_sdcc_status();
		#endif	
		#ifdef LTE_CRASH_DUMP
			
				printk("[LTE_ASSERT] lte_ers_panic_prev dump start\n");
				lte_panic_report_prev(packet_buff,real_packet_size);
		#endif			
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
		
				printk("[LTE_ASSERT] lte_ers_panic dump start\n");
				lte_crash_log(packet_buff,real_packet_size,NULL);
			
				kfree(packet_buff);
			}
#ifdef LTE_CRASH_DUMP
		}
#endif
	}
/* END: 0013575 seongmook.yim@lge.com 2011-01-05 */   
	else
	{	/* Invalid HIM block type */
		LTE_ERROR("Invalid HIM block type! 0x%x\n", him_blk_type);
	}
/* END: 0012603 jaegyu.lee@lge.com 2010-12-17 */
	gLte_sdio_info->him_rx_cnt++;

	error = 0;

err:
	return error;
}
/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
#ifdef LTE_CRASH_DUMP
int lte_panic_report_prev(unsigned char* lte_log_buf_prev, unsigned int size)
{
	FUNC_ENTER();
	int fd_dump;
	int fd_write;
	int fd_close;
	int fd_fs;
	
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(get_ds());

	printk("[LTE_ASSERT] lte_panic_report_prev size = %d\n",size);
	
	fd_dump = sys_open("/data/lte_ers_panic_prev", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_dump < 0) {
		printk("%s : can't open the file\n", __func__);
		return;
	}

	fd_write=sys_write(fd_dump, lte_log_buf_prev, size);
	if (fd_write < 0) {
		printk("%s : can't write the file\n", __func__);
		return;
	}

	fd_close=sys_close(fd_dump);
	if (fd_close < 0) {
		printk("%s : can't close the file\n", __func__);
		return;
	}

	sys_sync();
	set_fs(oldfs);
	FUNC_EXIT();	
}

int lte_panic_report_after(unsigned char* lte_log_buf_after, unsigned int size)
{
	FUNC_ENTER();
	int fd_dump;
	int fd_dump_read;
	int fd_write;
	int fd_read;
	int fd_close;
	int fd_fs;
	int i = 0;

	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(get_ds());

	msleep(100);
	printk("[LTE_ASSERT] lte_panic_report_after size = %d\n",size);

	fd_dump = sys_open("/data/lte_ers_panic_after", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_dump < 0) {
		printk("%s : can't open the file\n", __func__);
		return;
	}

	fd_write=sys_write(fd_dump, lte_log_buf_after, size);
	if (fd_write < 0) {
		printk("%s : can't write the file\n", __func__);
		return;
	}

	fd_close=sys_close(fd_dump);
	if (fd_close < 0) {
		printk("%s : can't close the file\n", __func__);
		return;
	}
	sys_sync();
	set_fs(oldfs);

	FUNC_EXIT();	
}
#endif /* LTE_CRASH_DUMP */
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */

EXPORT_SYMBOL(lte_him_register_cb_from_ved);
EXPORT_SYMBOL(lte_him_enqueue_ip_packet);

