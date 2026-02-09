BINARYDIR := debug

# Tool chain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

# Additional flags
PREPROCESSOR_MACROS := DEBUG
INCLUDE_DIRS := ./adi_inc ./adi_utils/inc \
				./apollo_api/public/inc ./apollo_api/public/src \
				./apollo_api/private/inc ./apollo_api/private/src \
				./apollo_api/private/inc/bitfields/b0

LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O0 -std=c99 -Wall -Werror
CXXFLAGS := -ggdb -ffunction-sections -O0 -std=c99 -Wall -Werror
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 
LINKER_SCRIPT := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

# Additional options detected from testing the tool chain
IS_LINUX_PROJECT := 1
