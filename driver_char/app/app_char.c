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
