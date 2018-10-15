
ifeq ($(PROJECT_DIR)/.config, $(wildcard $(PROJECT_DIR)/.config))
  include $(PROJECT_DIR)/.config
endif


ifeq ($(CONFIG_ARCH_ARM), y)
	ARCH := arm
endif
ifeq ($(CONFIG_ARCH_MIPS), y)
	ARCH := mips
endif

ifeq ($(CONFIG_CHIP_HI3516A), y)
	CROSS_COMPILE := arm-hisiv300-linux-
	CROSS_TOOLDIR = $(CONFIG_HI3516A_TOOLCHAIN_DIR)
endif
ifeq ($(CONFIG_CHIP_HI3518E), y)
	CROSS_COMPILE := arm-hisiv100-linux-
	CROSS_TOOLDIR = $(CONFIG_HI3518E_TOOLCHAIN_DIR)
endif

ifeq ($(CONFIG_PLATFORM_MSTAR), y)
  ifeq ($(CONFIG_CHIP_MSC313E), y)
	CROSS_COMPILE := arm-linux-gnueabihf-
  endif
  ifeq ($(CONFIG_CHIP_MSC316D), y)
	CROSS_COMPILE := arm-linux-gnueabihf-
  endif
  CROSS_TOOLDIR = $(CONFIG_MSCXX_TOOLCHAIN_DIR)
endif

ifeq ($(CONFIG_CHIP_INGT20), y)
	CROSS_COMPILE  := arm-linux-gnueabihf-
	CROSS_TOOLDIR = $(CONFIG_INGT20_TOOLCHAIN_DIR)
endif

ifeq ($(CONFIG_RELEASE_SAMPLE), y)
	BUILD_MODE := release
else
	BUILD_MODE := debug
endif

SAMPLE_VERSION := sample_$(CONFIG_BUILD_VER)

#build modules
MODULELIST :=
ifeq ($(CONFIG_MODULE_RTMP), y)
	MODULELIST += sample_rtmp
endif
ifeq ($(CONFIG_MODULE_TSUPLOAD), y)
	MODULELIST += sample_tsupload
endif
ifeq ($(CONFIG_MODULE_RTP), y)
	MODULELIST += sample_rtp
endif

