/*
 *  include/linux/mmc/sdio.h
 *
 *  Copyright 2006-2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#ifndef MMC_SDIO_H
#define MMC_SDIO_H

/* SDIO commands                         type  argument     response */
#define SD_IO_SEND_OP_COND          5 /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT            52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED          53 /* adtc [31:0] See below   R5  */

/*
 * SD_IO_RW_DIRECT argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] RAW flag
 *      [25:9] Register address
 *      [7:0] Data
 */

/*
 * SD_IO_RW_EXTENDED argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] Block mode
 *      [26] Increment address
 *      [25:9] Register address
 *      [8:0] Byte/block count
 */

/*
  SDIO status in R5
  Type
	e : error bit
	s : status bit
	r : detected and set for the actual command response
	x : detected and set during command execution. the host must poll
            the card by sending status command in order to read these bits.
  Clear condition
	a : according to the card state
	b : always related to the previous command. Reception of
            a valid command will clear it (with a delay of one command)
	c : clear by read
 */

#define R5_COM_CRC_ERROR	(1 << 15)	/* er, b */
#define R5_ILLEGAL_COMMAND	(1 << 14)	/* er, b */
#define R5_ERROR		(1 << 11)	/* erx, c */
#define R5_FUNCTION_NUMBER	(1 << 9)	/* er, c */
#define R5_OUT_OF_RANGE		(1 << 8)	/* er, c */
#define R5_STATUS(x)		(x & 0xCB00)
#define R5_IO_CURRENT_STATE(x)	((x & 0x3000) >> 12) /* s, b */

/*
 * Card Common Control Registers (CCCR)
 */

#define SDIO_CCCR_CCCR		0x00

#define  SDIO_CCCR_REV_1_00	0	/* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10	1	/* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_1_20	2	/* CCCR/FBR Version 1.20 */

#define  SDIO_SDIO_REV_1_00	0	/* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10	1	/* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20	2	/* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00	3	/* SDIO Spec Version 2.00 */

#define SDIO_CCCR_SD		0x01

#define  SDIO_SD_REV_1_01	0	/* SD Physical Spec Version 1.01 */
#define  SDIO_SD_REV_1_10	1	/* SD Physical Spec Version 1.10 */
#define  SDIO_SD_REV_2_00	2	/* SD Physical Spec Version 2.00 */

#define SDIO_CCCR_IOEx		0x02
#define SDIO_CCCR_IORx		0x03

#define SDIO_CCCR_IENx		0x04	/* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx		0x05	/* Function Interrupt Pending */

#define SDIO_CCCR_ABORT		0x06	/* function abort/card reset */

#define SDIO_CCCR_IF		0x07	/* bus interface controls */

#define  SDIO_BUS_WIDTH_1BIT	0x00
#define  SDIO_BUS_WIDTH_4BIT	0x02
#define  SDIO_BUS_WIDTH_8BIT  	0x03
#define  SDIO_BUS_ECSI		0x20	/* Enable continuous SPI interrupt */
#define  SDIO_BUS_SCSI		0x40	/* Support continuous SPI interrupt */

#define  SDIO_BUS_ASYNC_INT	0x20

#define  SDIO_BUS_CD_DISABLE     0x80	/* disable pull-up on DAT3 (pin 1) */

#define SDIO_CCCR_CAPS		0x08

#define  SDIO_CCCR_CAP_SDC	0x01	/* can do CMD52 while data transfer */
#define  SDIO_CCCR_CAP_SMB	0x02	/* can do multi-block xfers (CMD53) */
#define  SDIO_CCCR_CAP_SRW	0x04	/* supports read-wait protocol */
#define  SDIO_CCCR_CAP_SBS	0x08	/* supports suspend/resume */
#define  SDIO_CCCR_CAP_S4MI	0x10	/* interrupt during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_E4MI	0x20	/* enable ints during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_LSC	0x40	/* low speed card */
#define  SDIO_CCCR_CAP_4BLS	0x80	/* 4 bit low speed card */

#define SDIO_CCCR_CIS		0x09	/* common CIS pointer (3 bytes) */

/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND	0x0c
#define SDIO_CCCR_SELx		0x0d
#define SDIO_CCCR_EXECx		0x0e
#define SDIO_CCCR_READYx	0x0f

#define SDIO_CCCR_BLKSIZE	0x10

#define SDIO_CCCR_POWER		0x12

#define  SDIO_POWER_SMPC	0x01	/* Supports Master Power Control */
#define  SDIO_POWER_EMPC	0x02	/* Enable Master Power Control */

#define SDIO_CCCR_SPEED		0x13

#define  SDIO_SPEED_SHS		0x01	/* Supports High-Speed mode */
#define  SDIO_SPEED_EHS		0x02	/* Enable High-Speed mode */

/*
 * Function Basic Registers (FBR)
 */

#define SDIO_FBR_BASE(f)	((f) * 0x100) /* base of function f's FBRs */

#define SDIO_FBR_STD_IF		0x00

#define  SDIO_FBR_SUPPORTS_CSA	0x40	/* supports Code Storage Area */
#define  SDIO_FBR_ENABLE_CSA	0x80	/* enable Code Storage Area */

#define SDIO_FBR_STD_IF_EXT	0x01

#define SDIO_FBR_POWER		0x02

#define  SDIO_FBR_POWER_SPS	0x01	/* Supports Power Selection */
#define  SDIO_FBR_POWER_EPS	0x02	/* Enable (low) Power Selection */

#define SDIO_FBR_CIS		0x09	/* CIS pointer (3 bytes) */


#define SDIO_FBR_CSA		0x0C	/* CSA pointer (3 bytes) */

#define SDIO_FBR_CSA_DATA	0x0F

#define SDIO_FBR_BLKSIZE	0x10	/* block size (2 bytes) */

/*BEGIN: 0019235 seongmook.yim@lge.com 2011-07-07 */
/*MOD 0019235:  [LTE] Monitoring code & LTE Crash Dump Handling */
//#define SDCC_STATUS_MONITORING
#ifdef SDCC_STATUS_MONITORING

typedef enum T_SDCC_STATUS_INFO
{
	enum_lte_sdio_tty_write=0,
	enum_lte_sdio_wake_up_tx=1,
	enum_sdio_claim_host=2,
	enum_sdio_io_rw_ext_helper=3,
	enum_msmsdcc_print_status=4,
	enum_msmsdcc_reset_and_restore=5,
	enum_msmsdcc_request_end=6,
	enum_msmsdcc_stop_data=7,
	enum_msmsdcc_fifo_addr=8,
	enum_msmsdcc_delay=9,
	enum_msmsdcc_start_command_exec=10,
	enum_msmsdcc_dma_exec_func=11,
	enum_msmsdcc_dma_complete_tlet=12,
	enum_msmsdcc_dma_complete_func=13,
	enum_validate_dma=14,
	enum_msmsdcc_config_dma=15,
	enum_msmsdcc_start_command_deferred=16,
	enum_msmsdcc_start_data=17,
	enum_msmsdcc_start_command=18,
	enum_msmsdcc_data_err=19,
	enum_msmsdcc_pio_read=20,
	enum_msmsdcc_pio_write=21,
	enum_msmsdcc_pio_irq=22,
	enum_msmsdcc_irq=23,
	enum_msmsdcc_request_start=24,
	enum_msmsdcc_request=25,
	enum_msmsdcc_is_pwrsave=26,
	enum_msmsdcc_set_ios=27,
	enum_msmsdcc_set_pwrsave=28,
	enum_msmsdcc_get_ro=29,
	enum_msmsdcc_enable_sdio_irq=30,
	enum_msmsdcc_enable=31,
	enum_msmsdcc_disable=32,
	enum_msmsdcc_check_status=33,
	enum_msmsdcc_platform_status_irq=34,
	enum_msmsdcc_platform_sdiowakeup_irq=35,
	enum_msmsdcc_status_notify_cb=36,
	enum_msmsdcc_init_dma=37,
	enum_show_polling=38,
	enum_set_polling=39,
	enum_msmsdcc_early_suspend=40,
	enum_msmsdcc_late_resume=41,
	enum_msmsdcc_probe=42,
	enum_msmsdcc_remove=43,
	enum_msmsdcc_runtime_suspend=44,
	enum_msmsdcc_runtime_resume=45,
	enum_msmsdcc_runtime_idle=46,
	enum_msmsdcc_pm_suspend=47,
	enum_msmsdcc_pm_resume=48,
	enum_msmsdcc_init=49,
	enum_msmsdcc_exit=50,
	enum_mmc_io_rw_extended=51,
	enum_mmc_io_rw_direct=52,
	enum_mmc_set_data_timeout=53,
	enum_mmc_wait_for_req=54,
	enum_mmc_start_request=55,
	enum_lte_him_write_control_packet=56,
	enum_msmsdcc_max=57,
} T_SDCC_STATUS_INFO;


typedef struct _sdcc_status
{
	unsigned short tracking[30];
	int index;
}sdcc_status;
extern sdcc_status gLte_sdcc_status;

#endif /* SDCC_STATUS_MONITORING */
/*END: 0019235 seongmook.yim@lge.com 2011-07-07 */
#endif

