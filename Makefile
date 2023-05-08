BUILD_DIR = build
CORE_PATH = /z-uno2_core_03_00_12_beta05
ARM_GCC_PATH = /gcc
ARM_GCC_VERSION = 10.3.1
LIBCLANG_PATH = /libclang

DEB_VERSION := $(shell head -1 debian/changelog | awk '{print $$2}' | sed 's/[\(\)]//g')
SKETCH_VERSION := $(shell grep SKETCH_VERSION WbMsw.ino | awk -F= '{print $$2}')

all:
	@echo "Deb version: $(DEB_VERSION)"
	@echo "Sketch version: $(SKETCH_VERSION)"
	mkdir -p $(BUILD_DIR)
	# zme_make creates ~/ZMEStorage folder with ZUNOToolchain-*.log
	HOME=$(BUILD_DIR) zme_make build WbMsw.ino \
		-S $(CORE_PATH)/cores \
		-S $(CORE_PATH)/libraries \
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
