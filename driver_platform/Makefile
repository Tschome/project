CROSS_COMPILE ?= XXX-linux-gnu-

ENV_KERNEL_DIR ?= $(PWD)/../../kernel
KDIR := ${ENV_KERNEL_DIR}

MODULE_NAME := platform_demo

all: modules

.PHONY: modules clean

EXTRA_CFLAGS += -I$(PWD)/include

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := platform.o

$(MODULE_NAME)-objs += boards/boards_info.o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@find . -name '*.o' -o -name '*~' -o -name '.depend' -o -name '.*.cmd' \
    -o -name '*.mod.c' -o -name '.tmp_versions' -o -name '*.ko' \
    -o -name '*.symvers' -o -name 'modules.order' | xargs rm -rf
