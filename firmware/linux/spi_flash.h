#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include "ets_sys.h"

uint32 spi_flash_get_id(void);
int spi_flash_erase_sector(uint16 sec);
int spi_flash_write(uint32 des_addr, uint32 *src_addr, uint32 size);
int spi_flash_read(uint32 src_addr, uint32 *des_addr, uint32 size);

#endif
