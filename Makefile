FQBN := arduino:mbed_nano:nano33ble
PLATFORM := /Users/peter/Library/Arduino15/packages/arduino/hardware/mbed_nano/4.5.0
BOARD := nano33ble
CORE := arduino
CORE_DIR := $(PLATFORM)/cores/$(CORE)
API_DIR := $(CORE_DIR)/api
MBED_DIR := $(CORE_DIR)/mbed
DRIVERS_INC := $(MBED_DIR)/drivers/include
RTOS_INC := $(MBED_DIR)/rtos/include
CMSIS_DEVICE_INC := $(MBED_DIR)/cmsis/device
CMSIS_RTOS_INC := $(CMSIS_DEVICE_INC)/rtos/include
CMSIS_RTE_INC := $(CMSIS_DEVICE_INC)/RTE/include
CMSIS_RTOS2_INC := $(MBED_DIR)/cmsis/CMSIS_5/CMSIS/RTOS2/Include
CMSIS_RTX_INC := $(MBED_DIR)/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Include
CMSIS_RTX_CFG := $(MBED_DIR)/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Config
CMSIS_CORTEXM_INC := $(MBED_DIR)/cmsis/CMSIS_5/CMSIS/TARGET_CORTEX_M/Include
PLATFORM_INCDIR := $(MBED_DIR)/platform/include
PLATFORM_CXXDIR := $(MBED_DIR)/platform/cxxsupport
PLATFORM_LIBDIR := $(PLATFORM)/libraries
WIRE_INC := $(PLATFORM_LIBDIR)/Wire
VARIANT := ARDUINO_NANO33BLE
VARIANT_DIR := $(PLATFORM)/variants/$(VARIANT)
TARGET_NORDIC := $(MBED_DIR)/targets/TARGET_NORDIC
TARGET_ROOT := $(TARGET_NORDIC)/TARGET_NRF5x
TARGET_SDK := $(TARGET_ROOT)/TARGET_SDK_15_0
TARGET_UTIL := $(TARGET_SDK)/components/libraries/util
TARGET_NRFX := $(TARGET_SDK)/modules/nrfx
TARGET_I_NRFX := $(TARGET_SDK)/integration/nrfx
TARGET_MDK := $(TARGET_NRFX)/mdk
TARGET_HAL := $(TARGET_NRFX)/hal
TARGET_HAL_INC := $(MBED_DIR)/hal/include
TARGET_NRFS2 := $(TARGET_ROOT)/TARGET_NRF52
TARGET_MCU := $(TARGET_NRFS2)/TARGET_MCU_NRF52840
TARGET_CFG := $(TARGET_MCU)/config
TARGET_SUP := $(TARGET_MCU)/device
TARGET_DIR := $(TARGET_MCU)/TARGET_ARDUINO_NANO33BLE
TARGET_CRYPTO := $(MBED_DIR)/connectivity/drivers/mbedtls/FEATURE_CRYPTOCELL310/include/cryptocell310
MU_LIB_ROOT := ~/Documents/Arduino/libraries
TOOLCHAIN_ROOT := /Users/peter/Library/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4
TOOLCHAIN_BIN := $(TOOLCHAIN_ROOT)/bin

SRCS = sonic_screwdriver.cpp sonic_screwdriver_model_data.cpp
OBJS = $(addsuffix .o,$(basename $(SRCS)))
CC := $(TOOLCHAIN_BIN)/arm-none-eabi-gcc
CXX := $(TOOLCHAIN_BIN)/arm-none-eabi-g++
RM := /bin/rm

CFLAGS = -O
CXXFLAGS =  -c -w -g3 -nostdlib @/Users/peter/Library/Arduino15/packages/arduino/hardware/mbed_nano/4.5.0/variants/ARDUINO_NANO33BLE/defines.txt @/Users/peter/Library/Arduino15/packages/arduino/hardware/mbed_nano/4.5.0/variants/ARDUINO_NANO33BLE/cxxflags.txt -DARDUINO_ARCH_NRF52840 -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -w -x c++ -E -CC -DARDUINO=10607 -DARDUINO_ARDUINO_NANO33BLE -DARDUINO_ARCH_MBED_NANO -DARDUINO_ARCH_MBED \
-I $(PLATFORM)/cores/arduino -I $(VARIANT_DIR)  -I $(API_DIR) -I $(RTOS_INC) -I $(CMSIS_RTOS_INC) \
-I $(CMSIS_RTOS2_INC) -I $(CMSIS_RTX_INC) -I $(CMSIS_RTX_INC)1 -I $(RTOS_INC)/rtos -I $(CMSIS_DEVICE_INC) -I $(CMSIS_RTE_INC) \
-I $(DRIVERS_INC) -I $(PLATFORM_CXXDIR) -I $(CMSIS_RTX_CFG) -I $(CMSIS_CORTEXM_INC) \
-I $(PLATFORM_INCDIR) -I $(PLATFORM_INCDIR)/platform  -I $(WIRE_INC) -I $(TARGET_NORDIC) -I $(TARGET_ROOT) \
-I $(TARGET_SDK)/TARGET_SOFTDEVICE_NONE/nrf_soc_nosd -I $(TARGET_UTIL) -I $(TARGET_CFG) \
-I $(TARGET_NRFX) -I $(TARGET_NRFX)/drivers/include -I $(TARGET_I_NRFX) -I $(TARGET_NRFS2) -I $(TARGET_MCU) -I $(TARGET_HAL) -I $(TARGET_HAL_INC) \
-I $(TARGET_MDK) -I $(TARGET_SUP) -I $(TARGET_DIR) -I $(TARGET_CRYPTO) -I $(TARGET_CRYPTO)/internal \
-I $(MU_LIB_ROOT)/ArduinoBLE/src -I $(MU_LIB_ROOT)/Arduino_LSM9DS1/src \
-I $(MU_LIB_ROOT)/Harvard_TinyMLx/src \


.PHONY: clean all

all:	sonic_screwdriver.elf

sonic_screwdriver.elf: $(OBJS) sonic_screwdriver.ino

# Detectando las librerías usadas...
detect_lib:
	$(CXX) -c -w -g3 -nostdlib @$(VARIANT_DIR)/defines.txt @$(VARIANT_DIR)/cxxflags.txt -DARDUINO_ARCH_NRF52840 -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -w -x c++ -E -CC -DARDUINO=10607 -DARDUINO_ARDUINO_NANO33BLE -DARDUINO_ARCH_MBED_NANO -DARDUINO_ARCH_MBED -DARDUINO_LIBRARY_DISCOVERY_PHASE=1 -I$(PLATFORM)/cores/arduino -I$(CARIANT_DIR) -I$(PLATFORM)/cores/arduino/api/deprecated -I$(PLATFORM)/cores/arduino/api/deprecated-avr-comp -iprefix$(PLATFORM)/cores/arduino @$(VARIANT_DIR)/includes.txt /Users/peter/Library/Caches/arduino/sketches/0415236E2BBB132A1DF816E8A372C0A7/sketch/magic_wand.ino.cpp -o /dev/null

clean:
	$(RM) -f $(OBJS)

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

%.cpp: %.ino
	cp $< $@

# DO NOT DELETE
