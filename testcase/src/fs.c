#include "common.h"
#include "lib.h"
#include "sys.h"

int main()
{
	int i;
	int buf[256], tbuf[256];
	int fd = open("/testcase", 0, MODE_R | MODE_W);
	read(fd, buf, sizeof(buf));
	list("/");
	list("/usr/");
	list("/usr/john/");

	fd = open("/myfile", 0, MODE_R | MODE_W | MODE_G);
	for(i = 0; i < 256; i++)
		buf[i] = i;
	write(fd, buf, sizeof(buf));
	read(fd, tbuf, sizeof(tbuf));
/*	for(i = 0; i < 256; i++)
		printf("%x ", tbuf[i]);
	printf("\n");
*/	close(fd);
	list("/");

	return 0;
}
