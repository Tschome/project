### Helloworld入门驱动编写解析



如果你是初出茅庐的程序员,那么先阅读这一篇[<编写驱动前应该知道哪些>]()做一个预备,再阅读下面代码,然后你就可以豁然开朗了



helloworld.c文件

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include "include/common.h"

#define HELLOWORD_DRIVER_VERSION 20221111

static int times = 0;
module_param(times, int, S_IRUGO);
MODULE_PARM_DESC(times, "set this module times");


static int __init hello_init(void)
{
    printk("create hello module success!\n");

    return 0;
}

static void __exit hello_exit(void)
{
    printk("exit hello module success!\n");

}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("zywang");//声明是谁编写的该模块
MODULE_DESCRIPTION("xxx driver");//该模块是干嘛的描述
MODULE_LICENSE("GPL");//内核人事的特定许可

```



Makefile

```makefile
CROSS_COMPILE ?= arm-linux-gnu-

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

```

参考:

<linux设备驱动>

<深入Linux 设备驱动程序内核机制>

