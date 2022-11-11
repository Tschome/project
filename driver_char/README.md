# 虚拟字符设备编写及测试

##### char.c文件

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#include "include/common.h"

static struct cdev *chr_dev;//定义一个字符设备对象
static dev_t ndev;//字符设备节点设备号


static int chr_open(struct inode *nd, struct file *filp)
{
    int major = 0;
    int minor = 0;

    major = MAJOR(nd->i_rdev);
    minor = MINOR(nd->i_rdev);

    printk("chr_open:major = %d, minor = %d\n", major, minor);
    return 0;
}

static int chr_read(struct file *pFile, char __user *u, size_t size, loff_t *off)
{
    printk("chr_read success!\n");

    return 0;
}

struct file_operations chr_ops = {
    .owner = THIS_MODULE,
    .open = chr_open,
    .read = chr_read
};

static __init int char_init(void)
{
    int ret = 0;

    ret = alloc_chrdev_region(&ndev, 0, 1, "chr_dev");//动态分配设备号
    if(ret < 0){
        printk("alloc_chrdev_region failed!\n");
        return ret;
    }

    printk("init():major = %d, minor = %d\n", MAJOR(ndev), MINOR(ndev));//打印主设备号和次设备号

    chr_dev = kzalloc(sizeof(struct cdev), GFP_KERNEL);
	if(NULL == chr_dev){
		printk("kzalloc cdev failed!\n");
		goto err_alloc_cdev;
	}

    cdev_init(chr_dev, &chr_ops);//初始化字符设备对象,[加入系统前必须初始化]
    
    ret = cdev_add(chr_dev, ndev, 1);//将字符设备对象插入chr_dev注册进系统
    if(ret < 0){
        printk("cdev_add error!\n");
        goto err_cdev_add;//注册失败记得要销毁刚才分配的设备号
    }

    printk("create char driver success!\n");
    return 0;

err_cdev_add:
	kfree(chr_dev);

err_alloc_cdev:
	unregister_chrdev_region(ndev, 1);
    
    printk("create char driver failed!\n");
	return -1;

}

static __exit void char_exit(void)
{
    unregister_chrdev_region(ndev, 1);//释放分配的设备号
    cdev_del(chr_dev);//将字符设备对象chr_dev从系统中注销
    kfree(chr_dev);
	printk("exit char driver success!\n");
}


module_init(char_init);
module_exit(char_exit);

MODULE_AUTHOR("zywang");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("char driver demo");

```

##### Makefile:

```makefile
#交叉编译器配置
CROSS_COMPILE ?= XXX-linux-gnu-

ENV_KERNEL_DIR ?= $(PWD)/../../kernel
KDIR := ${ENV_KERNEL_DIR}

MODULE_NAME := char_demo

all: modules

.PHONY: modules clean
    
EXTRA_CFLAGS += -I$(PWD)/include

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := char.o

modules:
    @$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
    @find . -name '*.o' -o -name '*~' -o -name '.depend' -o -name '.*.cmd' \
    -o -name '*.mod.c' -o -name '.tmp_versions' -o -name '*.ko' \
    -o -name '*.symvers' -o -name 'modules.order' | xargs rm -rf

```

##### 应用程序

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define CHR_DEV_NAME "/dev/chr_dev"

int main()
{

    int ret = 0;
    char buf[32];
    int fd = 0;

    fd = open(CHR_DEV_NAME, O_RDONLY);
    if(fd < 0)
    {   
        printf("open file '%s' failed!\n", CHR_DEV_NAME);
        return -1; 
    }   

    read(fd, buf, 32);
    close(fd);

    return 0;
}
```

###### 编译app上层应用程序
    XX-linux-gcc app_char.c -o app_char


###### 测试程序
- 加载驱动
    ```c
    insmod char_demo.ko
    ```

    ==>界面会打印major和minor值,例如他们的值为:250, 0

- 创建设备节点
    ```c
    mknod /dev/chr_dev c 250 0
    ```

- 执行应用程序
    ```c
    ./app_char
    ```













#### struct file_operateions

```c
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
    ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
    int (*iterate) (struct file *, struct dir_context *);
    unsigned int (*poll) (struct file *, struct poll_table_struct *);
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    int (*open) (struct inode *, struct file *);
    int (*flush) (struct file *, fl_owner_t id); 
    int (*release) (struct inode *, struct file *);
    int (*fsync) (struct file *, loff_t, loff_t, int datasync);
    int (*aio_fsync) (struct kiocb *, int datasync);
    int (*fasync) (int, struct file *, int);
    int (*lock) (struct file *, int, struct file_lock *);
    ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
    unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
    int (*check_flags)(int);
    int (*flock) (struct file *, int, struct file_lock *);
    ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);    ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
    int (*setlease)(struct file *, long, struct file_lock **, void **);
    long (*fallocate)(struct file *file, int mode, loff_t offset,
              loff_t len);
    void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
    unsigned (*mmap_capabilities)(struct file *);
#endif
};
```

上述程序,你也许能发现,现实中的字符设备驱动程序的编写,其实**基本上是围绕如何实现 struct file_operation 中的那些函数指针成员而展开**的.

通过内核文件系统在其间穿针引线,应用程序对文件类函数--函数指针的调用,如read(),将最终被转接到 struct file_operations 中对应函数指针的具体实现上.

该结构体中唯一非函数指针成员 `owner` , 标识当前 struct file_operateions 对象所属的内核模块,几乎所有的设备驱动程序都会用 THIS_MODULE 宏给 owner赋值

```c
#include <linux/module.h>
#define THIS_MODULE (&__this_module)
```

`__this_module` 是内核模块的编译工具链为当前模块产生的 struct moduleleiixng的类型对象,所以 THIS_MODULE 实际上是当前模块对象的指针, **file_operateion 中的owner 成员可以避免 file_operateions中的函数正在被调用时,其所属的模块被系统中卸载掉**.若被编译进内核, THIS_MODULE 被赋值NULL,无任何作用.



#### 字符设备的内核抽象

```c
struct cdev {
    struct kobject kobj;//内核对象
    struct module *owner;//字符设备驱动程序所在的内核模块对象指针
    const struct file_operations *ops;
    struct list_head list;//系统中字符设备形成的链表
    dev_t dev;//设备号,包含主设备号和次设备号
    unsigned int count;//隶属于同一主设备号的次设备号的个数,用于标识由当前设备驱动程序控制的同类设备的数量
};
```



#### 设备号构成

linux系统中一个设备号由主设备号和次设备号构成,Linux 内核用主设备号来顶为对应的设备驱动程序 , 而次设备号则由驱动程序使用,用来标识它所管理的若干同类设备.因此,从这个角度而言,设备号作为设备资源需要严格管理,防止因设备号的混乱所导致系统里设备的混乱.

linux 用 dev_t 类型变量来标识一个设备号,它是 unsigned int类型:

```c
<include/linux/types.h>
typedef __u32 __kernel_dev_t;
typedef __kernel_dev_t  dev_t;
```

在内核2.6.39中,dev_t 的低20位表示次设备号,高12位代表主设备号.

```c
<include/linux/kdev_t.h>
#define MAJOR(dev)	((unsigned int)((dev)>>MINORBITS))//提取主设备号
#define MINOR(dev)	((unsigned int)((dev) & MINIRMASK))//提取次设备号
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))//合并主次设备号
```

#### 设备号分配管理

```c
<linux/fs.h>
int register_chrdev_region(dev_t first, unsigned int count, char *name);
//first 要分配的起始设备号,他的次设备号一般是从0开始
//count 申请次设备号的数量
//name	连接到这个编号范围的设备的名字
```

register_chrdev_region 函数第三个参数**`name`它会出现在 `/proc/devices`和` sysfs `中**.

如果你确实事先知道你需要哪个设备编号, register_chrdev_region 工作得好. 然而, 你常常不会知道你的设备使用哪个主编号; 在 Linux 内核开发社团中一直努力使用动态分配设备编号. 内核会乐于动态为你分配一个主编号, 但是你必须使用一个不同的函数来请求这个分配.

```c
int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);
```

不管你任何分配你的设备编号, 你应当在不再使用它们时释放它. 设备编号的释放使用:

```c
void unregister_chrdev_region(dev_t first, unsigned int count);
```

调用 unregister_chrdev_region 的地方常常是你的模块的 cleanup 函数.

###### 主设备号的动态分配

略,详见<linux设备驱动程序>P35

#### 字符设备的注册

把一个字符设备加入到系统中所需调用的函数为`cdev_add`.

#### 设备文件节点的生成

Linux系统所有的设备文件都位于 /dev 目录下.当Linux内核在挂载完根文件系统后,会在这个根文件系统 /dev 目录上重新挂载一个芯的文件系统 devtmpfs.

在 /dev 目录下用 mknod命令来创建一个新的设备文件节点 demodev , 对应的驱动程序使用的设备主设备号为2, 次设备号为0,命令如下:

```shell
mknod /dev/demodev c 2 0
```

























