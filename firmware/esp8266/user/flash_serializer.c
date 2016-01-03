#include "serdes.h"
#include "flashutils.h"
#include "crc.h"
#include "protos.h"

#define FLASH_MAGIC 0xF14E

#define DEBUGFLASH(x...) os_printf(x)

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
    uint8_t erased;
    uint8_t validated;
} flash_serializer_ctx_t;

LOCAL flash_serializer_ctx_t flash_serializer_ctx;

#define FLASHCTX(x) ((flash_serializer_ctx_t*)x->pvt)

LOCAL int flash_get_first_sector()
{
    return CHUNK1_SECTOR_START +
        ( ALIGN(read_current_irom0_size(), SECTORSIZE)>>SECTORBITS );
}

void ICACHE_FLASH_ATTR flash_ser_initialize(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = &flash_serializer_ctx;
    me->pvt = ctx;

    ctx->startsector = ctx->currentsector = flash_get_first_sector();

    ctx->erased = 0;
    ctx->sectoroffset = sizeof(config_header_t);
    ctx->validated = 0;
    ctx->crc = 0;

    os_printf("Start of flash area: 0x%08x\n",  ctx->startsector<<SECTORBITS);
}

LOCAL int ICACHE_FLASH_ATTR flash_validate(flash_serializer_ctx_t *ctx)
{
    config_header_t header;
    uint16_t crc = 0;
    unsigned char chunk[64];
    unsigned offset = flash_get_first_sector() << SECTORBITS;

    spi_flash_read( offset, (void*)&header, sizeof(config_header_t));
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
        spi_flash_read( offset, (void*)chunk, size);
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

LOCAL int flash_do_write(flash_serializer_ctx_t *ctx, const uint8_t*data, unsigned size)
{
    if ( !ctx->erased ) {
        // Erase current sector.
        if (spi_flash_erase_sector( (uint16)ctx->currentsector) <0) {
            return -1;
        }
        ctx->erased = 1;
    }

    crc16_update_buffer(&ctx->crc, data,size);

    if (spi_flash_write( (ctx->currentsector<<SECTORBITS) + ctx->sectoroffset, (uint32*)data, size)<0) {
        return -1;
    }
    ctx->sectoroffset+=size;

    if (ctx->sectoroffset>=(1<<SECTORBITS)) {
        ctx->erased = 0;
        ctx->sectoroffset=0;
        ctx->currentsector++;
    }
    return 0;
}

int flash_ser_write(struct serializer_t *me, const void *data, unsigned size)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);

    if (size==0)
        return 0;


    // TODO: Check if we overflow.

    return flash_do_write( ctx, data, size );

}

int flash_ser_read(struct serializer_t *me, void *data, unsigned size)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);

    if (!ctx->validated) {
        if (flash_validate(ctx)<0)
            return -1;
    }

    int r = spi_flash_read((ctx->currentsector<<SECTORBITS) + ctx->sectoroffset, data, size );
    if (r>=0) {
        ctx->sectoroffset+=size;
        ctx->currentsector+= (ctx->sectoroffset & ~((1<<SECTORBITS)-1));
        ctx->sectoroffset &= ((1<<SECTORBITS)-1);
    }
    return r;
}

void flash_ser_finalise(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    config_header_t header;

    header.magic = FLASH_MAGIC;

    header.size  = ((ctx->currentsector-ctx->startsector)<<SECTORBITS) +
        ctx->sectoroffset - sizeof(config_header_t);
    DEBUGFLASH("Data size: %d\n", header.size);
    header.datacrc = ctx->crc;

    header.headercrc = crc16_calc( (uint8_t*)&header, sizeof(config_header_t)-sizeof(uint16_t));

    spi_flash_write(ctx->startsector<<SECTORBITS, (uint32*)&header, sizeof(header));
}


void flash_ser_rewind(struct serializer_t *me)
{
    flash_serializer_ctx_t *ctx = FLASHCTX(me);
    ctx->currentsector = ctx->startsector;
    ctx->sectoroffset = sizeof(config_header_t);
}

void flash_ser_truncate(struct serializer_t *me)
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
};
