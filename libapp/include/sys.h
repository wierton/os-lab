#ifndef __SYS_H__
#define __SYS_H__

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define MODE_R (1 << 0)
#define MODE_W (1 << 1)
#define MODE_G (1 << 2)

int open(const char *pathname, int flags, int mode);
int close(int fd);
uint32_t read(int fd, void *buf, uint32_t count);
uint32_t write(int fd, const void *buf, uint32_t count);
int lseek(int fd, int offset, int whence);

int list(char *path);

#endif
