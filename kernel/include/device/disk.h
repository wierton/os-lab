#ifndef __DISK_H__
#define __DISK_H__

void read_section(uint32_t dst, int sectnum);
int read_disk(void *dst, uint32_t offset, uint32_t size);

#endif
