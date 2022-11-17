#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

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

	platform_set_drvdata(pdev, ptr);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(NULL == res){
		printk("get device resources failed!\n");
		goto err_get_res_failed;
	}

	if(!request_mem_region(res->start, resource_size(res), pdev->name)){
		printk("request_mem_region failed!\n");
		goto err_request_mem;
	}

	iomem = ioremap(res->start, resource_size(res));
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


