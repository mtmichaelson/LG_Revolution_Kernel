#CONFIG_MACH_LGE_BRYCE	sungmin.shin	10.06.22
#create new file.

EXTRA_CFLAGS += -Iarch/arm/mach-msm

# BEGIN: 0009214 sehyuny.kim@lge.com 2010-09-03
# MOD 0009214: [DIAG] LG Diag feature added in side of android
# sangwoo.kang 2010-09-01 S
obj-$(CONFIG_LGE_SUPPORT_RAPI) += LG_rapi_client.o 
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_testmode.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_testmode_sysfs.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_fw_diag_class.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_fw_diag_communication.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_event_log.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_keypress.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_screen_shot.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_udm.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_fw_diag_mtc.o
obj-$(CONFIG_LGE_DIAGTEST) += lg_diag_wifi.o
# sangwoo.kang 2010-09-01 E
# END: 0009214 sehyuny.kim@lge.com 2010-09-03

obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce.o
obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce-keypad.o
ifeq ($( BUILD_LG_HW_REV),2)
	obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce-touchkey.o
endif
#CONFIG_MACH_LGE_BRYCE	sungwoo.cho	10.04.20
#For Vibrator Setting
obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce-misc.o
#CONFIG_MACH_LGE_BRYCE	ehgrace.kim	10.04.21
#For Audio
obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce-headset.o
#CONFIG_MACH_LGE_BRYCE	chanhee.park	10.04.21
#For Camera
obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce-camera.o
#CONFIG_MACH_LGE_BRYCE chanha.park@lge.com    10.08.26
#For Bluetooth
obj-$(CONFIG_MACH_LGE_BRYCE) += board-bryce-bt.o
#obj-$(CONFIG_MACH_LGE) += devices_lge.o
# BEGIN: 0010090 sehyuny.kim@lge.com 2010-10-21
# MOD 0010090: [FactoryReset] Enable Recovery mode FactoryReset
obj-$(CONFIG_MACH_LGE) += lge_emmc_direct_access.o
# END: 0010090 sehyuny.kim@lge.com 2010-10-21
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_cmd.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_eta.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_mmc.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_event_log.o
obj-$(CONFIG_LGE_ATS_UEVENT_DEVICE) += lge_ats_uevent.o
obj-$(CONFIG_LGE_ATS_INPUT_DEVICE) += lge_ats_input.o # ATS_INPUT_DEVICE
obj-$(CONFIG_LGE_DIAG_WMC) += lg_diag_wmc.o
#Baikal ID 0009963 CONFIG_MACH_LGE_BRYCE	kiran.kanneganti@lge.com 13-oct-2010
#For Audio
obj-$(CONFIG_MACH_LGE_BRYCE) += lge_audio_misc_ctrl.o
# kwangdo.yi for ERS handler 10.10.26 S
# 0010357: add ram dump for ERS in AP side
# kwangdo.yi@lge.com 10.11.29 S
# 0011521: [kernel] ERS enable/disable feature added
obj-$(CONFIG_ANDROID_RAM_CONSOLE) += lge_errorhandler.o
# kwangdo.yi@lge.com  10.11.04
# 0010459: file name change from board-swift-ers.c to lge-bryce-ers.c
# kwangdo.yi@lge.com 10.11.29 S
# 0011521: [kernel] ERS enable/disable feature added
obj-$(CONFIG_ANDROID_RAM_CONSOLE) += lge-bryce-ers.o
# kwangdo.yi E
# // BEGIN 0011665: eundeok.bae@lge.com FTM MODE FOR ONLY KERNEL BOOTING
# // [KERNEL] Added source code For Sleep Mode Test, Test Mode V8.3 [250-42] 
obj-y += lg_ftmmode.o
# // END 0011665: eundeok.bae@lge.com FTM MODE FOR ONLY KERNEL BOOTING
# 0014609 : neo.kang@lge.com to save the autoall test result
obj-y += lge_bryce_temp.o
