## 编译app上层应用程序
	XX-linux-gcc app_char.c -o app_char


## 测试程序
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
