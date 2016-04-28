#include "serdes.h"
#include "flashutils.h"
#include "crc.h"
#include "protos.h"
#include "alloc.h"
#include "osapi.h"
#include "user_interface.h"
#include "flash.h"
#include "error.h"

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
    flash_control_t fc;
} flash_serializer_ctx_t;

LOCAL flash_serializer_ctx_t flash_serializer_ctx;

#define FLASHCTX(x) ((flash_serializer_ctx_t*)x->pvt)

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

int ICACHE_FLASH_ATTR flash_ser_initialize(struct serializer_t *me, int pos)
{
    flash_serializer_ctx_t *ctx = &flash_serializer_ctx;
    me->pvt = ctx;
    if (pos<0)
        pos = flash_get_first_sector();
    ctx->startsector = ctx->currentsector = (unsigned)pos;
    ctx->erased = 0;
    ctx->sectoroffset = sizeof(config_header_t);
    ctx->validated = 0;
    ctx->crc = 0;

    flash_control_init( &ctx->fc );
    os_printf("Start of flash area: 0x%08x\n",  ctx->startsector<<SECTORBITS);
    return 0;
}

LOCAL int flash_validate(flash_serializer_ctx_t *ctx)
{
    config_header_t header;
    uint16_t crc = 0;
    unsigned char chunk[64];
    unsigned offset = (unsigned)ctx->startsector << SECTORBITS;

    spiflash_read_cached(&ctx->fc, offset, (void*)&header, sizeof(config_header_t));
    if (header.magic != FLASH_MAGIC) {
        DEBUGFLASH("Invalid flash magic %04x\n", (unsigned)header.magic);
        return EINVALIDMAGIC;
    }

    crc = crc16_calc( (uint8_t*)&header, sizeof(config_header_t)-sizeof(uint16_t));

    if (crc != header.headercrc) {
        DEBUGFLASH("Invalid flash header crc 0x%04x (calculated 0x%04x)\n", (unsigned)header.headercrc,
                  crc);
        return EINVALIDCRC;
    }

    uint16_t len = header.size;
    offset += sizeof(config_header_t);
    crc = 0;

    while (len) {
        unsigned size = len>64 ? 64:len;
        spiflash_read_cached(&ctx->fc, offset, (void*)chunk, size);
        crc16_update_buffer(&crc, chunk, size);
        len-=size;
        offset+=size;
    }
    if (crc!=header.datacrc) {
        DEBUGFLASH("Invalid flash data crc 0x%04x (calculated 0x%04x)\n", (unsigned)header.datacrc,
                  crc);
        return EINVALIDCRC;
    }
    ctx->validated = 1;

    return NOERROR;
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

    if (spiflash_write_cached( &ctx->fc, (ctx->currentsector<<SECTORBITS) + ctx->sectoroffset, data, size)<0) {
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
    int r;
    if (!ctx->validated) {
        r = flash_validate(ctx);
        if (r<0)
            return r;
    }

    r = spiflash_read_cached(&ctx->fc,(ctx->currentsector<<SECTORBITS) + ctx->sectoroffset, data, size );
    if (r>=0) {
        ctx->sectoroffset+=size;
        ctx->currentsector+= (ctx->sectoroffset & ~((1<<SECTORBITS)-1));
        ctx->sectoroffset &= ((1<<SECTORBITS)-1);
    }
    return r;
}

LOCAL int ICACHE_FLASH_ATTR flash_ser_finalise(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    config_header_t header;

    header.magic = FLASH_MAGIC;

    header.size  = ((ctx->currentsector-ctx->startsector)<<SECTORBITS) +
        ctx->sectoroffset - sizeof(config_header_t);
    DEBUGFLASH("Data size: %d\n", header.size);
    DEBUGFLASH("Data CRC: %04x\n", ctx->crc);
    header.datacrc = ctx->crc;

    header.headercrc = crc16_calc( (uint8_t*)&header, sizeof(config_header_t)-sizeof(uint16_t));

    spiflash_write_cached(&ctx->fc, ctx->startsector<<SECTORBITS, (uint8_t*)&header, sizeof(header));

    spiflash_flush(&ctx->fc);
    return NOERROR;
}

LOCAL int ICACHE_FLASH_ATTR flash_ser_release(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    flash_control_release(&ctx->fc);
    return 0;
}


LOCAL int ICACHE_FLASH_ATTR flash_ser_rewind(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    ctx->currentsector = ctx->startsector;
    ctx->sectoroffset = sizeof(config_header_t);
    return NOERROR;
}

LOCAL int ICACHE_FLASH_ATTR flash_ser_truncate(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    ctx->crc = 0;
    ctx->erased = 0;
    return flash_ser_rewind(me);
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
