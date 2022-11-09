#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include "include/common.h"

static struct cdev chr_dev;
static dev_t ndev;


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

	ret = alloc_chrdev_region(&ndev, 0, 1, "chr_dev");
	if(ret < 0){
		printk("alloc_chrdev_region failed!\n");
		return ret;
	}

	printk("init():major = %d, minor = %d\n", MAJOR(ndev), MINOR(ndev));

	cdev_init(&chr_dev, &chr_ops);
	ret = cdev_add(&chr_dev, ndev, 1);
	if(ret < 0){
		printk("cdev_add error!\n");
		goto err_cdev_add;
	}

	printk("create char driver success!\n");
	return 0;

err_cdev_add:
	unregister_chrdev_region(ndev, 1);
	return -1;
}

static __exit void char_exit(void)
{
	printk("exit char driver success!\n");
	cdev_del(&chr_dev);
	unregister_chrdev_region(ndev, 1);
}


module_init(char_init);
module_exit(char_exit);

MODULE_AUTHOR("zywang");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("char driver demo");


