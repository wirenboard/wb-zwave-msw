BUILD_DIR = build
## Clone https://github.com/Z-Wave-Me/Z-Uno-G2-Core
# CORE_PATH = ~/Z-Uno-G2-Core
## Download http://rus.z-wave.me/files/z-uno/g2/tc/arm-none-eabi-gcc-7_2_4-linux64.tar.gz
# ARM_GCC_PATH = ~/gcc
# ARM_GCC_VERSION = 7.2.1
## Download http://rus.z-wave.me/files/z-uno/g2/tc/libclang_11_0_1-linux64.tar.gz
# LIBCLANG_PATH = ~/libclang

all:
	mkdir $(BUILD_DIR)
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
