# platform设备驱动编写及编译



### platform.c

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>//platform设备驱动头文件

#include "include/common.h"

struct resource *res;
void __iomem *iomem;

static int XX_platform_probe(struct platform_device *pdev)
{
	void *ptr;
	ptr = kzalloc(sizeof(int) * 100, GFP_KERNEL);
	if(NULL == ptr){
		printk("kzalloc mem failed!\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, ptr);//存储在probe申请的空间,以免丢失

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);//从设备中获取相关资源
	if(NULL == res){
		printk("get device resources failed!\n");
		goto err_get_res_failed;
	}

	if(!request_mem_region(res->start, resource_size(res), pdev->name)){//根据获取到的物理资源给他申请虚拟地址空间
		printk("request_mem_region failed!\n");
		goto err_request_mem;
	}

	iomem = ioremap(res->start, resource_size(res));//将申请到的虚拟地址空间映射为CPU可访问的
	if(!iomem){
		printk("ioremap failed!\n");
		goto err_ioremap;
	}

	return 0;

err_ioremap:
	release_mem_region(res->start, resource_size(res));
err_request_mem:
err_get_res_failed:
	kfree(ptr);

	return -1;
}

static int __exit XX_platform_remove(struct platform_device *pdev)
{
	void *ptr = platform_get_drvdata(pdev);
	
	iounmap(iomem);
	release_mem_region(res->start, resource_size(res));
	kfree(ptr);
	return 0;
}

extern struct platform_device XX_platform_device;
static struct platform_driver XX_platform_driver  ={
	.probe = XX_platform_probe,
	.remove = __exit_p(XX_platform_remove),
	.driver = {
		.name = "plaform_dev",
		.owner = THIS_MODULE,
		.suspend = NULL,
		.resume = NULL,
		.pm = NULL,
	},
};

static __init int platform_init(void)
{
	int ret = 0;

	ret = platform_device_register(&XX_platform_device);
	if(ret != 0){
		printk("register platform device failed!\n");
		return ret;
	}

	ret = platform_driver_register(&XX_platform_driver);
	if(ret != 0){
		printk("register platform driver failed!\n");
	}

	return ret;
}

static __exit void platform_exit(void)
{
	platform_driver_unregister(&XX_platform_driver);
	platform_device_unregister(&XX_platform_device);
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("zywang");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("platform driver demo");

```

注:

> `platform_get_resource`在使用板载资源时,遇到多个资源时,每次获取资源时,`platform_get_resource`函数接口永远从资源列表的第一个开始检索



### 板载资源boards_info.c

```c
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/i2c-gpio.h>
#include <soc/gpio.h>
#include <soc/base.h>

static struct resource temp_resources[] = {
	[0] = {
			.start = 0x0000000,//该处为虚拟出的资源,请按照实际修改
			.end = 0x00000800,
			.flags = IORESOURCE_MEM,
	},
};

struct platform_device XX_platform_device = {
	.name = "platform",
	.id = -1,
	.dev = {

	},
	.resource = temp_resources,
	.num_resources = ARRAY_SIZE(temp_resources),
};
```

### Makefile

```makefile
CROSS_COMPILE ?= 啊让吗-linux-gnu-

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

```



------

# platform设备驱动

## 概述

在`Linux2.6`的设备驱动模型里，关注的是总线、设备、驱动这三个实体，总线作为桥梁将设备和驱动绑定。**在系统每注册一个设备的时候，会寻找与之匹配的驱动；相反的，在系统每注册一个驱动时，会寻找与之匹配的设备，而匹配由总线完成**。

总线作为驱动和设备的桥梁，对于本身依附于`PCI`、`USB`、`IIC`、`SPI`等设备而言，这不成问题。但是在嵌入式系统里，`soc`系统中所集成的独立外设控制器、挂靠在`soc`内存空间的外设等却不依靠这些总线。

更加通俗的说`platform总线`是区别于`USB`、`SPI`、`I2C`的虚拟总线，**所有直接通过内存寻址的设备都映射到这条虚拟总线上**来。

基于以上需求，即对Linux的设备驱动模型进行了封装，封装模拟出`platform平台总线`，相对应的设备为`platform_device`，相对应的驱动为`platform_driver`。

## platform特性

#### platform总线的优点：

- 通过`platform`总线，可以遍历所有挂载在`platform`总线上的设备。
- 实现设备与驱动的分离，通过`platform`总线，设备与驱动是分开注册的，因为有`probe`函数，可以随时检测与设备匹配的驱动，匹配成功的就会把这个驱动向内核注册；
- 一个驱动可以供同类的几个设备使用，这个功能的实现是因为驱动注册过程中有一个遍历设备的操作。



## platform总线原型

bus_type结构体：总线结构体

```c
/* include/linux/device.h */
  
struct bus_type {
    const char *name;                                  /* 总线名字 */
    const char *dev_name; 
    struct device *dev_root;
    struct device_attribute *dev_attrs;
    const struct attribute_group **bus_groups;         /* 总线属性 */
    const struct attribute_group **dev_groups;         /* 设备属性 */
    const struct attribute_group **drv_groups;         /* 驱动属性 */
  
    int (*match)(struct device *dev, struct device_driver *drv);      /* 设备驱动匹配函数 */
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
    int (*probe)(struct device *dev);
    int (*remove)(struct device *dev);
    void (*shutdown)(struct device *dev);
  
    int (*online)(struct device *dev);
    int (*offline)(struct device *dev);
    int (*suspend)(struct device *dev, pm_message_t state);
    int (*resume)(struct device *dev);
    const struct dev_pm_ops *pm;
    const struct iommu_ops *iommu_ops;
    struct subsys_private *p;
    struct lock_class_key lock_key;
};
```

**platform 总线是 bus_type 类型的常量**，之所以说它是常量是因为这个变量已经被 Linux 内核赋值好了，其结构体成员对应的函数也已经在内核里面写好。

定义如下：

```c
/* drivers/base/platform.c */
  
struct bus_type platform_bus_type = {
    .name = "platform",
    .dev_groups = platform_dev_groups,
    .match = platform_match,       /* 匹配函数 */
    .uevent = platform_uevent,
    .pm = &platform_dev_pm_ops,
};
```

由此可以看出platform总线是Linux设备驱动总线的一个**封装**，不同于其他总线的。



## platform驱动

```c
/*
 * platform_driver结构体：（include/linux/platform_device.h）
*/
struct platform_driver {
    int (*probe)(struct platform_device *);    //和device_driver中是相同功能，但一般使用那个，而不是用probe
    int (*remove)(struct platform_device *);//卸载驱动，device_driver中也有一个，使用哪一个需要研究
    void (*shutdown)(struct platform_device *);
    int (*suspend)(struct platform_device *, pm_message_t state);
    int (*resume)(struct platform_device *);
    struct device_driver driver;                /* 驱动基类 内置的device_driver结构体*/
    const struct platform_device_id *id_table;  /* id_table表 该设备驱动支持的设备的列表，通过这个指针指向platform_device_id类型的数组*/
    
};
```

`platform_driver`中`const struct platform_device_id *id_table`是`id_table`表，在 `platform` 总线匹配驱动和设备时 `id_table` 表匹配法时使用的，这个 `id_table` 表其实是一个数组，里面的每个元素类型都为 `platform_device_id`，`platform_device_id` 是一个结构体，内容如下：

```c
struct platform_device_id {
    char name[PLATFORM_NAME_SIZE];
    kernel_ulong_t driver_data;
};
```

 `platform_driver` 中 `driver` 是一个驱动基类，相当于驱动具有的最基础的属性，在不同总线下具有的属性则存放在 `platform_driver` 结构体下。

​    驱动基类结构体 `device_driver` 内容为：

```c
/* include/linux/device.h */
  
struct device_driver {
    const char *name;                               /* platform 总线来匹配设备与驱动的第四种方        
                                                       法就是直接粗暴匹配两者的 name 字段 */
    struct bus_type *bus;
    struct module *owner;
    const char *mod_name; 
    bool suppress_bind_attrs; 
    const struct of_device_id *of_match_table;      /* 采用设备树时驱动使用的的匹配表 */
    const struct acpi_device_id *acpi_match_table;
    int (*probe) (struct device *dev);
    int (*remove) (struct device *dev);
    void (*shutdown) (struct device *dev);
    int (*suspend) (struct device *dev, pm_message_t state);
    int (*resume) (struct device *dev);
    const struct attribute_group **groups;
    const struct dev_pm_ops *pm;
    struct driver_private *p;
};
```

`driver` 中 `of_match_table` 也是一个匹配表，这个匹配表是 `platform 总线`给驱动和设备做匹配时使用设备树匹配时用的，也是一个数组，数组元素都为 `of_device_id` 类型，该类型结构体如下：

```c
/* include/linux/mod_devicetable.h */
  
struct of_device_id {
    char name[32];
    char type[32];
    char compatible[128];   /* 使用设备树匹配时就是把设备节点的 compatible 属性值和 of_match_table 中
                               每个项目的这个 compatible 作比较，如果有相等的就表示设备和驱动匹配成功 */
    const void *data;
};
```

### 

## platform设备

```c
/*
 * platform_device结构体：（include/linux/platform_device.h）
*/
struct platfrom_device{
	const char *name;		//平台设备的名字
    int id;		//ID是用来区分如果设备名字相同的时候（通过在后面添加一个数字来代表不同的设备）
    struct devide dev;		//内置的device结构体
    u32 num_resource;		//资源结构体数量
    struct resource *resource;	//指向一个资源结构体数组
    const struct platform_device_id *id_entry;//用来进行余设备驱动匹配用的id_table表
    /*arch specific additions*/
    struct pdev_archdata archdata;//自留 添加自己的东西
};
```



platform_device结构体中的`struct resource`分析：

```c
struct resource{
    resource_size_t start;//资源的起始值，如果是地址，他是物理地址，不是虚拟地址
    resource_size_t end;//资源的结束值，如果是地址，他是物理地址，不是虚拟地址
    const char *name;//资源名
    unsigned long flags;//资源的标示，用来识别不同的资源
    struct resource *parent, *sibling, *child;//资源指针，可构成链表
    
};
```

## platform设备和驱动的匹配

第一种匹配方式，`OF` 类型的匹配，也就是设备树采用的匹配方式，`of_driver_match_device` 函数定义在文件`include/linux/of_device.h` 中。`device_driver` 结构体(表示设备驱动)中有个名为`of_match_table` 的成员变量，此成员变量保存着驱动的`compatible` 匹配表，设备树中的每个设备节点的`compatible` 属性会和`of_match_table` 表中的所有成员比较，查看是否有相同的条目，如果有的话就表示设备和此驱动匹配，设备和驱动匹配成功以后`probe` 函数就会执行。

第二种匹配方式，`ACPI `匹配方式。

第三种匹配方式，`id_table `匹配，每个`platform_driver` 结构体有一个`id_table`成员变量，顾名思义，保存了很多id 信息。这些id 信息存放着这个platform 驱动所支持的驱动类型。

第四种匹配方式，如果第三种匹配方式的`id_table `不存在的话就直接比较驱动和设备的`name` 字段，看看是不是相等，如果相等的话就匹配成功。

对于支持设备树的Linux 版本号，一般设备驱动为了兼容性都支持设备树和无设备树两种匹配方式。也就是第一种匹配方式一般都会存在，第三种和第四种只要存在一种就可以，一般用的最多的还是第四种，也就是直接比较驱动和设备的`name` 字段，毕竟这种方式最简单了。


## platform设备资源和数据

### 资源定义

`platform`设备资源由`resource`结构体来描述的。

```c
struct resource{
    resource_size_t start;//资源的起始值，如果是地址，他是物理地址，不是虚拟地址
    resource_size_t end;//资源的结束值，如果是地址，他是物理地址，不是虚拟地址
    const char *name;//资源名
    unsigned long flags;//资源的标示，用来识别不同的资源
    struct resource *parent, *sibling, *child;//资源指针，可构成链表
};
```

我们通常关注`start`、`end`和`flags`三个字段，分别标明资源的开始值、结束值和类型，flags可以为`IORESOURCE_IO`、`IORESOURCE_MEM`、`IORESOURCE_IRQ`、`IORESOURCE_DMA`等。

> start和end的含义会随着flags的变化而变化：
>
> flags为IORESOURCE_MEM：start和end分别表示platform_device占据内存的开始和结束地址，
>
> flags为IORESOURCE_IRQ：start和end分别表示platform_device使用的中断号的开始和结束值，

`IORESOURCE_MEM`对于同类型的资源而说，可以有多份，例如设备占据2个内存区域，那么可以定义2个`IORESOURCE_MEM`资源。

`IORESOURCE_IRQ`如果使用一个中断号，则`start`和`end`值相同



### 资源获取

`resource`的定义常在`BSP板文件`中进行，而在具体的设备驱动中透过`platform_get_resource()`这样的API来获取。

```c
static struct resource s3c_i2c_resource[] = {
	[0] = {
		.start = S3C24XX_PA_IIC,
		.end   = S3C24XX_PA_IIC + S3C24XX_SZ_IIC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_IIC,
		.end   = IRQ_IIC,
		.flags = IORESOURCE_IRQ,
	}
};
```

用下面两个函数即可取出资源

```c
iic_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
```

对于IRQ而言，`platform_get_resource()`还有一个封装变体`platform_get_irq()`，在`platform_get_irq()`内部实际上调用的就是`platform_get_source(dev, IORESOURCE_IRQ, num)`;

其实，设备除了可以在BSP中定义资源以外，还可以附加一些数据信息，因为对设备的硬件描述除了中断、内存、DMA通道以外，可能还会有一些配置信息，而这些信息也是依赖于板信息的（不适合直接放在设备驱动里的）。platform提供了platform_data自定义数据的支持。如

![image-20220817161747179](/home/user/share/md/linux/driver/2022-08-15-设备驱动 · 第7篇《platform设备驱动》.assets/image-20220817161747179.png)

