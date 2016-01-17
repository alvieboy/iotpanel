#include "serdes.h"
#include "flashutils.h"
#include "crc.h"
#include "protos.h"
#include "alloc.h"

#define FLASH_MAGIC 0xF14F

#define DEBUGFLASH(x...) /*os_printf(x)*/

typedef struct {
    uint16_t magic;
    uint16_t size;
    uint16_t datacrc;
    uint16_t headercrc;
} config_header_t;//__attribute__((packed));

typedef struct {
    uint16_t startsector;
    uint16_t currentsector;
    uint16_t sectoroffset;
    uint16_t crc;
    uint8_t erased:1;
    uint8_t validated:1;
    uint8_t cached_dirty:1;
    int8_t cached_sector_number;
    uint8_t *cached_sector;
} flash_serializer_ctx_t;

LOCAL flash_serializer_ctx_t flash_serializer_ctx;

#define FLASHCTX(x) ((flash_serializer_ctx_t*)x->pvt)

LOCAL int spiflash_read_cached(flash_serializer_ctx_t *ctx, unsigned address, uint8_t *target, unsigned size);

LOCAL int flash_get_first_sector()
{
    return CHUNK1_SECTOR_START +
        ( ALIGN(read_current_irom0_size(), SECTORSIZE)>>SECTORBITS );
}
#if 0
LOCAL void byte_memcpy(uint8_t *dest, const uint8_t *src, unsigned size)
{
    while (size) {
        *dest++ = *src++;
    }
}
#endif

void ICACHE_FLASH_ATTR flash_ser_initialize(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = &flash_serializer_ctx;
    me->pvt = ctx;

    ctx->startsector = ctx->currentsector = flash_get_first_sector();
    ctx->cached_sector_number = -1;
    ctx->cached_sector = os_malloc(SECTORSIZE);
    ctx->erased = 0;
    ctx->sectoroffset = sizeof(config_header_t);
    ctx->validated = 0;
    ctx->crc = 0;
    ctx->cached_dirty = 0;

    os_printf("Start of flash area: 0x%08x\n",  ctx->startsector<<SECTORBITS);
}

LOCAL int flash_validate(flash_serializer_ctx_t *ctx)
{
    config_header_t header;
    uint16_t crc = 0;
    unsigned char chunk[64];
    unsigned offset = flash_get_first_sector() << SECTORBITS;

    spiflash_read_cached(ctx, offset, (void*)&header, sizeof(config_header_t));
    if (header.magic != FLASH_MAGIC) {
        DEBUGFLASH("Invalid flash magic %04x\n", (unsigned)header.magic);
        return -1;
    }

    crc = crc16_calc( (uint8_t*)&header, sizeof(config_header_t)-sizeof(uint16_t));

    if (crc != header.headercrc) {
        DEBUGFLASH("Invalid flash header crc 0x%04x (calculated 0x%04x)\n", (unsigned)header.headercrc,
                  crc);
        return -2;
    }

    uint16_t len = header.size;
    offset += sizeof(config_header_t);
    crc = 0;

    while (len) {
        unsigned size = len>64 ? 64:len;
        spiflash_read_cached(ctx, offset, (void*)chunk, size);
        crc16_update_buffer(&crc, chunk, size);
        len-=size;
        offset+=size;
    }
    if (crc!=header.datacrc) {
        DEBUGFLASH("Invalid flash data crc 0x%04x (calculated 0x%04x)\n", (unsigned)header.datacrc,
                  crc);
        return -3;
    }
    ctx->validated = 1;

    return 0;
}

LOCAL int spiflash_flush(flash_serializer_ctx_t *ctx)
{
    if (ctx->cached_sector_number>0 && ctx->cached_dirty) {
        DEBUGFLASH("Writing out sector %d\n",ctx->cached_sector_number);
        spi_flash_write( ctx->cached_sector_number<<SECTORBITS, (uint32*)ctx->cached_sector, SECTORSIZE);
        ctx->cached_sector_number = -1;
        ctx->cached_dirty = 0;
    }
    return 0;
}

LOCAL int spiflash_read_cached_single(flash_serializer_ctx_t *ctx, unsigned address, uint8_t *target, unsigned size)
{
    uint16_t sect = (address>>SECTORBITS);
    unsigned offset = (address & ((1<<SECTORBITS)-1));

    if (ctx->cached_sector_number != sect ) {
        spiflash_flush(ctx);
        ctx->cached_sector_number = sect;
        spi_flash_read( (unsigned)sect<<SECTORBITS, (uint32*)ctx->cached_sector, SECTORSIZE);
        ctx->cached_dirty = 0;
    }
    os_memcpy( target, ctx->cached_sector+offset, size);
    return 0;
}


LOCAL int spiflash_read_cached(flash_serializer_ctx_t *ctx, unsigned address, uint8_t *target, unsigned size)
{
    do {
        unsigned offset = (address & ((1<<SECTORBITS)-1));
        unsigned toread;

        if ((size+offset) > SECTORSIZE) {
            toread = SECTORSIZE - offset;
        } else {
            toread = size;
        }
        if (spiflash_read_cached_single(ctx, address, target, toread)<0)
            return -1;
        address += toread;
        size -= toread;
    } while (size);
    return 0;
}


LOCAL int spiflash_write_cached(flash_serializer_ctx_t *ctx, unsigned address, const uint8_t *data, unsigned size)
{
    uint16_t sect = (address>>SECTORBITS);
    unsigned offset = (address & ((1<<SECTORBITS)-1));
    if (ctx->cached_sector_number != sect ) {
        spiflash_flush(ctx);
        // Read it
        DEBUGFLASH("Cacheing sector %d at %08x buffer %08x\n",sect, (unsigned)sect<<SECTORBITS,
                   (unsigned)ctx->cached_sector
                  );

        spi_flash_read( (unsigned)sect<<SECTORBITS, (uint32*)ctx->cached_sector, SECTORSIZE);
        ctx->cached_sector_number = sect;
    }
    DEBUGFLASH("Copy %08x -> %08x (%d)\n", data, ctx->cached_sector + offset, size);
    os_memcpy( ctx->cached_sector + offset, data, size );
    ctx->cached_dirty = 1;
    DEBUGFLASH("Cached\n");
    
    return 0;
}


LOCAL int flash_do_write(flash_serializer_ctx_t *ctx, const uint8_t*data, unsigned size)
{
    if ( !ctx->erased ) {
        // Erase current sector.
        DEBUGFLASH("Erasing sector %d\n",(uint16)ctx->currentsector);
        if (spi_flash_erase_sector( (uint16)ctx->currentsector) <0) {
            DEBUGFLASH("Error erasing flash\n");
            return -1;
        }
        ctx->erased = 1;
        DEBUGFLASH("Erased successfully\n");
    }

    crc16_update_buffer(&ctx->crc, data,size);

    DEBUGFLASH("Write %08x\n", (ctx->currentsector<<SECTORBITS) + ctx->sectoroffset);

    if (spiflash_write_cached( ctx, (ctx->currentsector<<SECTORBITS) + ctx->sectoroffset, data, size)<0) {
        DEBUGFLASH("Error in write\n");
        return -1;
    }
    DEBUGFLASH("Write OK\n");
    ctx->sectoroffset+=size;

    if (ctx->sectoroffset>=(1<<SECTORBITS)) {
        ctx->erased = 0;
        ctx->sectoroffset=0;
        ctx->currentsector++;
    }
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR flash_ser_write(struct serializer_t *me, const void *data, unsigned size)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);

    if (size==0)
        return 0;


    // TODO: Check if we overflow.

    return flash_do_write( ctx, data, size );

}

LOCAL int ICACHE_FLASH_ATTR flash_ser_read(struct serializer_t *me, void *data, unsigned size)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);

    if (!ctx->validated) {
        if (flash_validate(ctx)<0)
            return -1;
    }

    int r = spiflash_read_cached(ctx,(ctx->currentsector<<SECTORBITS) + ctx->sectoroffset, data, size );
    if (r>=0) {
        ctx->sectoroffset+=size;
        ctx->currentsector+= (ctx->sectoroffset & ~((1<<SECTORBITS)-1));
        ctx->sectoroffset &= ((1<<SECTORBITS)-1);
    }
    return r;
}

LOCAL void ICACHE_FLASH_ATTR flash_ser_finalise(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    config_header_t header;

    header.magic = FLASH_MAGIC;

    header.size  = ((ctx->currentsector-ctx->startsector)<<SECTORBITS) +
        ctx->sectoroffset - sizeof(config_header_t);
    DEBUGFLASH("Data size: %d\n", header.size);
    DEBUGFLASH("Data CRC: %04x\n", header.size);
    header.datacrc = ctx->crc;

    header.headercrc = crc16_calc( (uint8_t*)&header, sizeof(config_header_t)-sizeof(uint16_t));

    spiflash_write_cached(ctx, ctx->startsector<<SECTORBITS, (uint8_t*)&header, sizeof(header));

    spiflash_flush(ctx);
}

LOCAL void ICACHE_FLASH_ATTR flash_ser_release(struct serializer_t *me)
{
     flash_serializer_ctx_t *ctx = FLASHCTX(me);
     if (ctx->cached_sector)
         os_free(ctx->cached_sector);
}


LOCAL void ICACHE_FLASH_ATTR flash_ser_rewind(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    ctx->currentsector = ctx->startsector;
    ctx->sectoroffset = sizeof(config_header_t);
}

LOCAL void ICACHE_FLASH_ATTR flash_ser_truncate(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    ctx->crc = 0;
    ctx->erased = 0;
    flash_ser_rewind(me);
}




struct serializer_t flash_serializer = {
    .write = &flash_ser_write,
    .read = &flash_ser_read,
    .rewind = &flash_ser_rewind,
    .finalise = &flash_ser_finalise,
    .truncate = &flash_ser_truncate,
    .initialize = &flash_ser_initialize,
    .release = &flash_ser_release
};
