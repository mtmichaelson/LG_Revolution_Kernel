#Android makefile to build kernel as a part of Android Build

ifeq ($(TARGET_PREBUILT_KERNEL),)

KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
KERNEL_CONFIG := $(KERNEL_OUT)/.config
##Mocana IPSec fix...2011.2.25...
ETC_DIR :=$(TARGET_OUT)/etc
SBIN_DIR :=$(TARGET_ROOT_OUT)/sbin
DATA_DIR :=$(TARGET_ROOT_OUT)/data
ETC_ROOT=system/core/rootdir/etc
MOCANA_IPSEC_ROOT=vendor/mocana/mocana_ipsec
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage
KERNEL_HEADERS_INSTALL := $(KERNEL_OUT)/usr

ifeq ($(TARGET_USES_UNCOMPRESSED_KERNEL),true)
$(info Using uncompressed kernel)
TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/piggy
else
TARGET_PREBUILT_KERNEL := $(TARGET_PREBUILT_INT_KERNEL)
endif

file := $(TARGET_OUT)/lib/modules/oprofile.ko
ALL_PREBUILT += $(file)
$(file) : $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(transform-prebuild-to-target)

#LGE_CHANGE_S, [cheolsook.lee@lge.com], 2011-02-25, [MS910] cp wireless.ko to system/lib/modules>
KERNEL_MODULES_OUT := $(TARGET_OUT)/lib/modules
KERNEL_MODULES_OUT_LIB : = $(KERNEL_MODULES_OUT)/modules/
BOARD_WIRELESS_CHIP := bcm4329
#LGE_CHANGE_E, [cheolsook.lee@lge.com], 2011-02-25, [MS910] cp wireless.ko to system/lib/modules>

$(KERNEL_OUT):
	mkdir -p $(KERNEL_OUT)
#LGE_CHANGE_S, [cheolsook.lee@lge.com], 2011-02-25, [MS910] cp wireless.ko to system/lib/modules>
$(KERNEL_MODULES_OUT):
	mkdir -p $(KERNEL_MODULES_OUT)
#LGE_CHANGE_E, [cheolsook.lee@lge.com], 2011-02-25, [MS910] cp wireless.ko to system/lib/modules>

$(KERNEL_CONFIG): $(KERNEL_OUT)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- $(KERNEL_DEFCONFIG)

$(KERNEL_OUT)/piggy : $(TARGET_PREBUILT_INT_KERNEL)
	$(hide) gunzip -c $(KERNEL_OUT)/arch/arm/boot/compressed/piggy.gzip > $(KERNEL_OUT)/piggy

$(TARGET_PREBUILT_INT_KERNEL): $(KERNEL_OUT) $(KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL)
	echo "YD at point 2"
#	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-  CONFIG_DEBUG_SECTION_MISMATCH=y
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-  
	echo "YD at point 3"
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- modules
# Mocana added for Nanosec
	echo "YD about to build Nanosec"
	cd $(MOCANA_IPSEC_ROOT)&&  ./build.ipsec&& cd ..
#LGE_CHANGE_S, [cheolsook.lee@lge.com], 2011-02-25, [MS910] cp wireless.ko to system/lib/modules>
# Mocana added 2 KOs also
	mkdir -p $(TARGET_OUT)/lib
	mkdir -p $(KERNEL_MODULES_OUT)
#START_LGE  [Integration] mkdir directory to support cloud
	mkdir -p $(SBIN_DIR)
#END_LGE
	echo "YD at point 4"
	cp -f $(KERNEL_OUT)/drivers/net/wireless/$(BOARD_WIRELESS_CHIP)/wireless.ko $(KERNEL_MODULES_OUT)
	cp -f $(MOCANA_IPSEC_ROOT)/bin/moc_memdrv.ko $(KERNEL_MODULES_OUT)
	cp -f $(MOCANA_IPSEC_ROOT)/bin/moc_ipsec.ko $(KERNEL_MODULES_OUT)
	cp -f $(MOCANA_IPSEC_ROOT)/bin/mkdev $(TARGET_ROOT_OUT_SBIN)
	cp -f $(ETC_ROOT)/mocana_ipsec.sh $(ETC_DIR)
	chmod +x $(ETC_DIR)/mocana_ipsec.sh
	chmod 700 $(TARGET_ROOT_OUT_SBIN)/mkdev
#LGE_CHANGE_E, [dongp.kim@lge.com], 2010-08-10, cp wireless.ko to system/lib/modules>

$(KERNEL_HEADERS_INSTALL): $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- headers_install

kerneltags: $(KERNEL_OUT) $(KERNEL_CONFIG)
	echo "YD at point 5"
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- tags

kernelconfig: $(KERNEL_OUT) $(KERNEL_CONFIG)
	echo "YD at point 6"
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- menuconfig
	cp $(KERNEL_OUT)/.config kernel/arch/arm/configs/$(KERNEL_DEFCONFIG)

endif
