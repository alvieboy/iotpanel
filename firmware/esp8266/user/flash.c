#include "flash.h"
#include "osapi.h"
#include "protos.h"
#include "alloc.h"

#define DEBUGFLASH(x...)  /* os_printf(x) */

void ICACHE_FLASH_ATTR flash_control_init(flash_control_t *ctrl)
{
    ctrl->cached_sector_number = -1;
    ctrl->cached_sector = os_malloc(SECTORSIZE);
    ctrl->cached_dirty = 0;
}

void ICACHE_FLASH_ATTR flash_control_release(flash_control_t *ctrl)
{
    if (ctrl->cached_sector)
        os_free(ctrl->cached_sector);
    ctrl->cached_sector = NULL;
}

LOCAL int spiflash_read_cached_single(flash_control_t *ctrl, unsigned address, uint8_t *target, unsigned size)
{
    DEBUGFLASH("Request flash read at address 0x%08x size %u\n",address,size);

    uint16_t sect = (address>>SECTORBITS);
    unsigned offset = (address & ((1<<SECTORBITS)-1));

    if (ctrl->cached_sector_number != sect ) {
        spiflash_flush(ctrl);
        ctrl->cached_sector_number = sect;
        spi_flash_read( (unsigned)sect<<SECTORBITS, (uint32*)ctrl->cached_sector, SECTORSIZE);
        ctrl->cached_dirty = 0;
    }


    os_memcpy( target, ctrl->cached_sector+offset, size);
#if 0
    {
        unsigned i;
        for (i=0;i<size;i++) {
            os_printf("%02x ",target[i]);
        }
    }
    os_printf("\n");
#endif
    return 0;
}

int spiflash_read_cached(flash_control_t *ctrl, unsigned address, uint8_t *target, unsigned size)
{
    do {
        unsigned offset = (address & ((1<<SECTORBITS)-1));
        unsigned toread;

        if ((size+offset) > SECTORSIZE) {
            toread = SECTORSIZE - offset;
            DEBUGFLASH("Partial read 0x%08x size %d\n", offset, toread);
        } else {
            toread = size;
        }
        if (spiflash_read_cached_single(ctrl, address, target, toread)<0)
            return -1;
        address += toread;
        size -= toread;
        target += toread;
    } while (size);
    return 0;
}

int spiflash_flush(flash_control_t *ctrl)
{
    if (ctrl->cached_sector_number>0 && ctrl->cached_dirty) {
        DEBUGFLASH("Writing out sector %d\n",ctrl->cached_sector_number);
        spi_flash_write( ctrl->cached_sector_number<<SECTORBITS, (uint32*)ctrl->cached_sector, SECTORSIZE);
        ctrl->cached_sector_number = -1;
        ctrl->cached_dirty = 0;
    }
    return 0;
}



int spiflash_write_cached(flash_control_t *ctrl, unsigned address, const uint8_t *data, unsigned size)
{
    uint16_t sect = (address>>SECTORBITS);
    unsigned offset = (address & ((1<<SECTORBITS)-1));
    if (ctrl->cached_sector_number != sect ) {
        spiflash_flush(ctrl);
        // Read it
/*        DEBUGFLASH("Cacheing sector %d at %08x buffer %08x\n",sect, (unsigned)sect<<SECTORBITS,
                   (unsigned)ctrl->cached_sector
                  );
  */
        spi_flash_read( (unsigned)sect<<SECTORBITS, (uint32*)ctrl->cached_sector, SECTORSIZE);
        ctrl->cached_sector_number = sect;
    }
    DEBUGFLASH("Copy %08x -> %08x (%d)\n", data, ctrl->cached_sector + offset, size);
    os_memcpy( ctrl->cached_sector + offset, data, size );
    ctrl->cached_dirty = 1;
    DEBUGFLASH("Cached\n");
    
    return 0;
}
