#include "common.h"
#include "lib.h"
#include "sys.h"

int main()
{
//	int i;
	int buf[64];
	int fd = open("/testcase", 0, MODE_R | MODE_W);
	read(fd, buf, sizeof(buf));
/*	for(i = 0; i < 64; i++)
		printf("%x ", buf[i]);
	printf("\n");
*/	close(fd);
	list("/");
	list("/usr/");
	list("/usr/john/");

	fd = open("/myfile", 0, MODE_R | MODE_W | MODE_G);
	close(fd);
	list("/");

	return 0;
}
