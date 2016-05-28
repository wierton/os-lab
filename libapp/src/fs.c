#include "common.h"
#include "syscall.h"
#include "sys.h"

int open(const char *pathname, int flags, int mode)
{
	return syscall(0x14, pathname, flags, mode);
}

int close(int fd)
{
	return syscall(0x15, fd);
}

uint32_t read(int fd, void *buf, uint32_t count)
{
	return syscall(0x16, fd, buf, count);
}

uint32_t write(int fd, const void *buf, size_t count)
{
	return syscall(0x17, fd, buf, count);
}

int list(char *path)
{
	return syscall(0x18, path);
}
