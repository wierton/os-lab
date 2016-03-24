#ifndef __DISK_H__
#define __DISK_H__

void read_section(uint32_t dst, int sectnum);
void read_disk(uint32_t dst, uint32_t offset, uint32_t size);

#endif
