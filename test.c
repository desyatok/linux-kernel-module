#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
	int dev = open("/dev/myrandom", O_RDWR, 0);
	if (dev == -1)
    {
        printf("No success\n");
        return -1;
    }
    int n = 20000;
    unsigned char buff[] = {4, 17, 133, 120, 117, 201, 45, 12, 240, 5};
    size_t written = write(dev,buff,10);
    printf("written: %lu\n", written - 1);
    unsigned char res[2 * n];
    size_t n_read = read(dev,res, n);

    if (n_read != n)
    {
        printf("couldn't read");
        return -1;
    }

    for (int i = 0; i < n; i++)
    {
        printf("%d ", res[i]);
    }

    printf("\n");
    close(dev);
	return 0;
}
