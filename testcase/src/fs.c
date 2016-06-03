#include "common.h"
#include "lib.h"
#include "sys.h"

int main()
{
	char buf[256] = "abcdefghijklmnopqrstuvwxyz", tbuf[256];
	list("/");
	list("/usr/");
	list("/usr/john/");

	int fd = open("/myfile", 0, MODE_R | MODE_W | MODE_G);
	write(fd, buf, sizeof(buf));
	read(fd, tbuf, sizeof(tbuf));
/*	for(i = 0; i < 256; i++)
		printf("%x ", tbuf[i]);
	printf("\n");
*/	close(fd);
	list("/");

	return 0;
}
