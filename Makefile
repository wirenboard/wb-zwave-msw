BUILD_DIR = build
CORE_PATH = /Z-Uno-G2-Core
ARM_GCC_PATH = /gcc
ARM_GCC_VERSION = 7.2.1
LIBCLANG_PATH = /libclang

# zme_make creates ~/ZMEStorage folder with ZUNOToolchain-*.log
export HOME=$(BUILD_DIR)

all:
	mkdir -p $(BUILD_DIR)
	zme_make build WbMsw.ino \
		-S $(CORE_PATH)/hardware/arduino/zunoG2/cores \
		-S $(CORE_PATH)/hardware/arduino/zunoG2/libraries \
		-S $(ARM_GCC_PATH)/lib/gcc/arm-none-eabi/$(ARM_GCC_VERSION)/include \
		-B $(BUILD_DIR) \
		-T $(ARM_GCC_PATH)/bin \
		-lc $(LIBCLANG_PATH) \
		-O make_listing \
		-O BO:-DARDUINO=152 \
		-O BO:-DARDUINO_ARCH_ZUNOG2 \
		-O BO:-Wno-register

install:
	install -Dm0644 $(BUILD_DIR)/WbMsw/WbMsw_ino_signed.bin $(DESTDIR)

clean:
	rm -fr $(BUILD_DIR)

.PHONY: all clean
