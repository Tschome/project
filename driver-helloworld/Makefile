CROSS_COMPILE ?= mips-v720s229-linux-gnu-

#模块编译依赖kernel
ENV_KERNEL_DIR ?= $(PWD)/../../kernel
KDIR := ${ENV_KERNEL_DIR}

#所要编译模块名称
MODULE_NAME := hello

all: modules

.PHONY: modules clean

EXTRA_CFLAGS += -I$(PWD)/include

#模块来自1个源文件helloworld.c
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := helloworld.o

# '@$(MAKE)' 不再将$(MAKE)指令输出到屏幕上
# '-C' 将当前工作目录转移到你所指定的位置
# 'M=$(PWD)' 返回到当前目录继续读入,执行当前Makefile
# '$@' 代表所有参数

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@find . -name '*.o' -o -name '*~' -o -name '.depend' -o -name '.*.cmd' \
    -o -name '*.mod.c' -o -name '.tmp_versions' -o -name '*.ko' \
    -o -name '*.symvers' -o -name 'modules.order' | xargs rm -rf
