TARGETNAME := apollo_main

#TARGETTYPE can be APP, STATIC or SHARED
TARGETTYPE := APP

EXCTL_FEATURE ?= 0
PLATFORM ?= ADS10
CONFIG ?= debug_b0
CONFIGURATION_FLAGS_FILE := $(CONFIG).mak
include $(CONFIGURATION_FLAGS_FILE)

#LINKER_SCRIPT defined inside the configuration file (e.g. debug.mak)
CONFIGURATION_LINKER_SCRIPT := $(LINKER_SCRIPT)

include $(ADDITIONAL_MAKE_FILES)

ifneq ($(CONFIGURATION_LINKER_SCRIPT),)
LINKER_SCRIPT := $(CONFIGURATION_LINKER_SCRIPT)
endif

ifneq ($(LINKER_SCRIPT),)
LDFLAGS += -T$(LINKER_SCRIPT)
endif

ifeq ($(BINARYDIR),)
error:
	$(error Invalid configuration, please check your inputs)
endif

SOURCEFILES := $(wildcard ./*.c) \
				$(wildcard ../ads10_apollo_ex_common/src/*.c) \
				$(wildcard ../../adi_utils/src/*.c) \
				$(wildcard ../../apollo_api/public/src/*.c) \
				$(wildcard ../../apollo_api/private/src/*.c) \
				$(wildcard ../../c_src/devices/fpga_apollo/public/src/*.c) \
				$(wildcard ../../c_src/devices/fpga_apollo/private/src/*.c) \
				$(wildcard ../../c_src/devices/adf4030/public/src/*.c) \
				$(wildcard ../../c_src/devices/adf4030/private/src/*.c) \
				$(wildcard ../../c_src/devices/adf4030/private/src/regmap/*.c) \
				$(wildcard ../../c_src/devices/adf4382/public/src/*.c) \
				$(wildcard ../../c_src/devices/adf4382/private/src/*.c) \
				$(wildcard ../../c_src/devices/adl6331/public/src/*.c) \
				$(wildcard ../../c_src/devices/adl6331/private/src/*.c) \
				$(wildcard ../../c_src/devices/adl6332/public/src/*.c) \
				$(wildcard ../../c_src/devices/adl6332/private/src/*.c) \
				$(wildcard ../../c_src/devices/hmc7044/public/src/*.c) \
				$(wildcard ../../c_src/devices/hmc7044/private/src/*.c) \
				$(wildcard ../../c_src/devices/ltc2977/public/src/*.c) \
				$(wildcard ../../c_src/devices/ltc2980/public/src/*.c) \
				$(wildcard ../../c_src/devices/ltm4681/public/src/*.c) \
				$(wildcard ../../c_src/image_loader/public/src/*.c) \
				$(wildcard ../../c_src/protocol/pmbus/src/*.c) \
				$(wildcard ../../c_src/protocol/smbus/src/*.c) \
				$(wildcard ../../c_src/math/vector/src/*.c) \
				$(wildcard ../../platforms/*.c) \
				$(wildcard ../../platforms/ads10/*.c) \
				$(wildcard ../../platforms/ads10/dma/*.c) \
				$(wildcard ../../platforms/ads10/smbus/*.c) \
				$(wildcard ../../platforms/linux_x86/*.c) \
				$(ADDITIONAL_SOURCEFILES)
				
EXTERNAL_LIBS :=
EXTERNAL_LIBS_COPIED := $(foreach lib, $(EXTERNAL_LIBS),$(BINARYDIR)/$(notdir $(lib)))

# Check if EXCTL FEATURE is enabled
ifeq ($(EXCTL_FEATURE), 1)
    # Commands or variables when EXCTL FEATURE is enabled
    CFLAGS += -DEXCTL_AUTOMATION
endif

CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))
CXXFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

CFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS) $(PLATFORM))
CXXFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS) $(PLATFORM))
ASFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))

LDFLAGS += $(addprefix -L,$(LIBRARY_DIRS))

ifeq ($(GENERATE_MAP_FILE),1)
LDFLAGS += -Wl,-Map=$(BINARYDIR)/$(basename $(TARGETNAME)).map
endif

LIBRARY_LDFLAGS = $(addprefix -l,$(LIBRARY_NAMES))

ifeq ($(IS_LINUX_PROJECT),1)
	RPATH_PREFIX := -Wl,--rpath='$$ORIGIN/../
	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)
	LIBRARY_LDFLAGS += -Wl,--rpath='$$ORIGIN'
	LIBRARY_LDFLAGS += $(addsuffix ',$(addprefix $(RPATH_PREFIX),$(dir $(EXTERNAL_LIBS))))

	ifeq ($(TARGETTYPE),SHARED)
		CFLAGS += -fPIC
		CXXFLAGS += -fPIC
		ASFLAGS += -fPIC
		LIBRARY_LDFLAGS += -Wl,-soname,$(TARGETNAME)
	endif

	ifneq ($(LINUX_PACKAGES),)
		PACKAGE_CFLAGS := $(foreach pkg,$(LINUX_PACKAGES),$(shell pkg-config --cflags $(pkg)))
		PACKAGE_LDFLAGS := $(foreach pkg,$(LINUX_PACKAGES),$(shell pkg-config --libs $(pkg)))
		CFLAGS += $(PACKAGE_CFLAGS)
		CXXFLAGS += $(PACKAGE_CFLAGS)
		LIBRARY_LDFLAGS += $(PACKAGE_LDFLAGS)
	endif
else
	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)
endif

LIBRARY_LDFLAGS += $(ADDITIONAL_LINKER_INPUTS)

all_make_files := $(firstword $(MAKEFILE_LIST)) $(CONFIGURATION_FLAGS_FILE) $(ADDITIONAL_MAKE_FILES)

ifeq ($(STARTUPFILES),)
	all_source_files := $(SOURCEFILES)
else
	all_source_files := $(STARTUPFILES) $(filter-out $(STARTUPFILES),$(SOURCEFILES))
endif

source_obj1 := $(all_source_files:.cpp=.o)
source_obj2 := $(source_obj1:.c=.o)
source_obj3 := $(source_obj2:.s=.o)
source_obj4 := $(source_obj3:.S=.o)
source_obj5 := $(source_obj4:.cc=.o)
source_objs := $(source_obj5:.cxx=.o)

all_objs := $(addprefix $(BINARYDIR)/, $(notdir $(source_objs)))

PRIMARY_OUTPUTS :=

ifeq ($(PRIMARY_OUTPUTS),)
PRIMARY_OUTPUTS := $(BINARYDIR)/$(TARGETNAME)
endif

all: $(PRIMARY_OUTPUTS)

ifeq ($(TARGETTYPE),APP)
$(BINARYDIR)/$(TARGETNAME): $(all_objs) $(EXTERNAL_LIBS)
	$(CC) -fprofile-arcs -ftest-coverage -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif

ifeq ($(TARGETTYPE),SHARED)
$(BINARYDIR)/$(TARGETNAME): $(all_objs) $(EXTERNAL_LIBS)
	$(LD) -shared -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif

ifeq ($(TARGETTYPE),STATIC)
$(BINARYDIR)/$(TARGETNAME): $(all_objs)
	$(AR) -r $@ $^
endif

-include $(all_objs:.o=.dep)

clean:
ifeq ($(USE_DEL_TO_CLEAN),1)
	cmd /C del /S /Q $(BINARYDIR)
else
	rm -rf ./$(BINARYDIR)
endif

$(BINARYDIR):
	mkdir $(BINARYDIR)

$(BINARYDIR)/%.o : ./%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../ads10_apollo_ex_common/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../adi_utils/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../apollo_api/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../apollo_api/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/fpga_apollo/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/fpga_apollo/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adf4030/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adf4030/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adf4030/private/src/regmap/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adf4382/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adf4382/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adl6331/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adl6331/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adl6332/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/adl6332/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/hmc7044/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/hmc7044/private/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/ltc2977/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/ltc2980/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/ltm4681/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/devices/image_loader/public/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/protocol/pmbus/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/protocol/smbus/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../c_src/math/vector/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../platforms/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../platforms/ads10/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../platforms/ads10/dma/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../platforms/ads10/smbus/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../../platforms/linux_x86/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
