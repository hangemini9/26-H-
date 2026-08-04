MSPM0_SDK_INSTALL_DIR ?= c:/ti/mspm0_sdk_2_10_00_04
TEST_MODE ?= 1

include $(MSPM0_SDK_INSTALL_DIR)/imports.mak

CC = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"
LNK = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"

SYSCFG_CMD = $(SYSCONFIG_TOOL) --compiler ticlang --product $(MSPM0_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_FILES := $(shell $(SYSCFG_CMD) --listGeneratedFiles --listReferencedFiles --output .. ../mspm0dache.syscfg)
SYSCFG_C_FILES = $(filter %.c,$(SYSCFG_FILES))
SYSCFG_H_FILES = $(filter %.h,$(SYSCFG_FILES))
SYSCFG_OPT_FILES = $(filter %.opt,$(SYSCFG_FILES))

# bluetooth.c is retained but temporarily excluded while JDY-31 is unplugged.
APP_SOURCES = \
    main.c \
    line_sensor.c \
    oled_display.c \
    question_timer.c \
    mc02_link.c \
    platform.c \
    motor.c \
    chassis.c \
    test_mode.c \
    imu.c \
    interrupts.c

CONFIG_SUFFIX = .test$(TEST_MODE)
APP_OBJECTS = $(patsubst %.c,%$(CONFIG_SUFFIX).obj,$(APP_SOURCES))
SYSCFG_OBJECTS = $(patsubst %.c,%$(CONFIG_SUFFIX).obj,$(notdir $(SYSCFG_C_FILES)))
OBJECTS = $(APP_OBJECTS) $(SYSCFG_OBJECTS)
DEPFILES = $(OBJECTS:.obj=.d)
CLEAN_OBJECTS = \
    $(patsubst %.c,%.test0.obj,$(APP_SOURCES)) \
    $(patsubst %.c,%.test1.obj,$(APP_SOURCES)) \
    $(patsubst %.c,%.test0.obj,$(notdir $(SYSCFG_C_FILES))) \
    $(patsubst %.c,%.test1.obj,$(notdir $(SYSCFG_C_FILES)))
CLEAN_DEPFILES = $(CLEAN_OBJECTS:.obj=.d)
LEGACY_OBJECTS = \
    $(APP_SOURCES:.c=.obj) \
    $(patsubst %.c,%.obj,$(notdir $(SYSCFG_C_FILES)))
LEGACY_DEPFILES = $(LEGACY_OBJECTS:.obj=.d)

ifeq ($(TEST_MODE),1)
NAME = mspm0dache
else
NAME = mspm0dache_release
endif

V := @
ifeq ($(VERBOSE),1)
V :=
endif

CFLAGS += \
    -I.. \
    -I. \
    $(addprefix @,$(SYSCFG_OPT_FILES)) \
    -DTEST_MODE=$(TEST_MODE) \
    -O0 \
    -gdwarf-3 \
    -mcpu=cortex-m0plus \
    -march=thumbv6m \
    -mfloat-abi=soft \
    -mthumb \
    -Wall \
    -Wextra \
    "-I$(MSPM0_SDK_INSTALL_DIR)/source/third_party/CMSIS/Core/Include" \
    "-I$(MSPM0_SDK_INSTALL_DIR)/source"

LFLAGS += \
    -ldevice.cmd.genlibs \
    "-L$(MSPM0_SDK_INSTALL_DIR)/source" \
    -L.. \
    ../device_linker.cmd \
    "-Wl,-m,$(NAME).map" \
    -Wl,--rom_model \
    -Wl,--warn_sections \
    "-L$(TICLANG_ARMCOMPILER)/lib" \
    -llibc.a

.PHONY: all clean syscfg

all: $(NAME).out

$(SYSCFG_FILES): syscfg
	@ echo SysConfig generation complete

syscfg: ../mspm0dache.syscfg
	@ echo Generating SysConfig files...
	$(V) $(SYSCFG_CMD) --output .. $<

define APP_C_RULE
$(basename $(notdir $(1)))$(CONFIG_SUFFIX).obj: ../$(1) $(SYSCFG_H_FILES)
	@ echo Building $$@
	$(V) $(CC) $(CFLAGS) -MMD -MP -MF $$(@:.obj=.d) -c $$< -o $$@
endef

$(foreach source,$(APP_SOURCES),$(eval $(call APP_C_RULE,$(source))))

define SYSCFG_C_RULE
$(basename $(notdir $(1)))$(CONFIG_SUFFIX).obj: $(1) $(SYSCFG_H_FILES)
	@ echo Building $$@
	$(V) $(CC) $(CFLAGS) -MMD -MP -MF $$(@:.obj=.d) -c $$< -o $$@
endef

$(foreach source,$(SYSCFG_C_FILES),$(eval $(call SYSCFG_C_RULE,$(source))))

$(NAME).out: $(OBJECTS)
	@ echo Linking $@
	$(V) $(LNK) -Wl,-u,_c_int00 $(OBJECTS) $(LFLAGS) -o $@

-include $(DEPFILES)

clean:
	@ echo Cleaning...
	-$(V) $(RM) $(CLEAN_OBJECTS) >$(DEVNULL) 2>&1
	-$(V) $(RM) $(CLEAN_DEPFILES) >$(DEVNULL) 2>&1
	-$(V) $(RM) $(LEGACY_OBJECTS) >$(DEVNULL) 2>&1
	-$(V) $(RM) $(LEGACY_DEPFILES) >$(DEVNULL) 2>&1
	-$(V) $(RM) mspm0dache.out >$(DEVNULL) 2>&1
	-$(V) $(RM) mspm0dache.map >$(DEVNULL) 2>&1
	-$(V) $(RM) mspm0dache_release.out >$(DEVNULL) 2>&1
	-$(V) $(RM) mspm0dache_release.map >$(DEVNULL) 2>&1
