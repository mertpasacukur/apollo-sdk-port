BINARYDIR := debug

# Tool chain
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CXX)
AR := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy

# Additional flags
override PREPROCESSOR_MACROS += DEBUG _GNU_SOURCE ADI_EN_STDERR
INCLUDE_DIRS := ../../adi_inc ../../adi_utils/inc  . \
				../../apollo_api/public/inc ../../apollo_api/public/src \
				../../apollo_api/private/inc ../../apollo_api/private/src ../../apollo_api/private/inc/bitfields/b0 \
				../../c_src/devices/fpga_apollo/public/include ../../c_src/devices/fpga_apollo/private/include \
				../../c_src/devices/image_loader/public/include ../../c_src/devices/image_loader/private/include\
				../../c_src/devices/adf4030/public/include ../../c_src/devices/adf4030/private/include ../../c_src/devices/adf4030/private/include/regmap \
				../../c_src/devices/adf4382/public/include ../../c_src/devices/adf4382/private/include ../../c_src/devices/adf4382/private/include/regmap \
				../../c_src/devices/adl6331/public/include ../../c_src/devices/adl6331/private/include ../../c_src/devices/adl6331/private/include/regmap \
				../../c_src/devices/adl6332/public/include ../../c_src/devices/adl6332/private/include ../../c_src/devices/adl6332/private/include/regmap \
				../../c_src/devices/hmc7044/public/include ../../c_src/devices/hmc7044/private/include \
				../../c_src/devices/ltc2977/public/include \
				../../c_src/devices/ltc2980/public/include \
				../../c_src/devices/ltm4681/public/include \
				../../c_src/protocol/pmbus/include \
				../../c_src/protocol/smbus/include \
				../../c_src/math/vector/inc \
				../../platforms \
				../../platforms/ads10 ../../platforms/ads10/dma ../../platforms/ads10/smbus \
				../../platforms/linux_x86 \
				../profiles/include \
				../ads10_apollo_ex_common/include


LIBRARY_DIRS :=
LIBRARY_NAMES :=
ADDITIONAL_LINKER_INPUTS := -lm
MACOS_FRAMEWORKS :=
LINUX_PACKAGES :=

CFLAGS := -ggdb -ffunction-sections -O0 -pedantic -std=gnu99 -Wall
CXXFLAGS := -ggdb -ffunction-sections -O0 -pedantic -std=gnu99 -Wall
ASFLAGS :=
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS :=
LINKER_SCRIPT :=

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

# Additional options detected from testing the tool chain
IS_LINUX_PROJECT := 1