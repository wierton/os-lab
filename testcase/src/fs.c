#include "common.h"
#include "lib.h"
#include "sys.h"

int main()
{
	char buf[256] = "abcdefghijklmnopqrstuvwxyz", tbuf[256];
	printf("-------------------------------1\n");
	list("/");
	printf("-------------------------------2\n");
	list("/usr/");
	printf("-------------------------------3\n");
	list("/usr/john/");
	printf("-------------------------------4\n");

	int fd = open("/myfile", 0, MODE_R | MODE_W | MODE_G);
	printf("-------------------------------5\n");
	write(fd, buf, sizeof(buf));
	printf("-------------------------------6\n");
	read(fd, tbuf, sizeof(tbuf));
	printf("-------------------------------7\n");
/*	for(i = 0; i < 256; i++)
		printf("%x ", tbuf[i]);
	printf("\n");
*/	close(fd);
	list("/");

	return 0;
}
