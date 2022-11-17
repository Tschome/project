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
			.start = 0x0000000,
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
