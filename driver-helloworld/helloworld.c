#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include "include/helloworld.h"

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

MODULE_AUTHOR("zywang");
MODULE_DESCRIPTION("xxx driver");
MODULE_LICENSE("GPL");




