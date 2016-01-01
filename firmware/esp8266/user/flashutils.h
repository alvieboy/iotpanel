#ifndef __FLASHUTILS_H__
#define __FLASHUTILS_H__

#include "spi_flash.h"

#define SECTORBITS 12
#define SECTORSIZE (1<<SECTORBITS)

#define RESERVED_SECTORS (16/4) /* 8KB reserved */

#define CHUNK0_SECTOR_START 0x0
#define CHUNK0_SIZE  0x40
#define CHUNK1_SECTOR_START 0x40 /* Sector where ITEXT starts */
#define CHUNK1_SIZE (0x40 - RESERVED_SECTORS) /* Sector where ITEXT starts */


extern char _irom0_text_start;
extern char _irom0_text_end;

extern int read_current_irom0_size();

#define ALIGN(x, alignment) (((x)+(alignment-1)) & ~(alignment-1))


#endif
