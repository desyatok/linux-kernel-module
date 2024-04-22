#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main()
{
	FILE *dev = fopen("/dev/myrandom", "w");
	if (dev == NULL)
	{
		printf("No success\n");
		return -1;
	}
    char *buf = "\x2\x17\x69\x6f\x77\x33";

    //int written = fprintf(dev,"%s", buf);
    size_t written = fwrite(buf, 1,strlen(buf) + 1,dev);
    printf("successfully wrote %ld\n", written);
    fclose(dev);
	return 0;
}

