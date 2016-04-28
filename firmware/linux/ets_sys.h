#ifndef __ETS_SYS__
#define __ETS_SYS__

#ifdef HOST

#include "os_type.h"

#define ICACHE_FLASH_ATTR
#define ICACHE_RODATA_ATTR
#define ICACHEFUN(name) name
#define LOCAL static

bool system_rtc_mem_read(uint8 src_addr, void *des_addr, uint16 load_size);
bool system_rtc_mem_write(uint8 des_addr, const void *src_addr, uint16 save_size);


#endif

#endif
