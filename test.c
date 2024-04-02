#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	int dev = open("/dev/aes_p", O_RDONLY);
	if (dev == -1) 
	{
		printf("No success\n");
		return -1;
	}
	printf("success\n");
	close(dev);
	return 0;
}

