#include "upgrade.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "alloc.h"

extern char _irom0_text_start;
extern char _irom0_text_end;

typedef struct {
    unsigned char magic;
    unsigned char num_segments;
    unsigned char dummy[2];
    uint32_t entry;
} header_entry_t;

typedef struct {
    uint32_t address;
    uint32_t size;
} segment_entry_t;

#define OTA_NUM_CHUNKS 2

#define CHUNK0_SECTOR_START 0x0
#define CHUNK0_SIZE  0x40
#define CHUNK1_SECTOR_START 0x40 /* Sector where ITEXT starts */
#define CHUNK1_SIZE 0x40 /* Sector where ITEXT starts */

#define RESERVED_SECTORS (8/4) /* 8KB reserved */

#define BLOCKSIZE 512

#define BLOCKS_PER_SECTOR (SECTORSIZE/BLOCKSIZE)

struct ota_chunk
{
    uint8_t start_sector;
    uint8_t available;
    uint8_t remaining;
    uint8_t erased:1;
    uint8_t block:7;    // 512-byte blocks.
};

static struct {
    struct ota_chunk chunks[OTA_NUM_CHUNKS];
    uint32_t start_address;
    int size;
    int current_chunk;
    int fwchunk;
} ota_state;

int read_current_irom0_size()
{
    const char *start = &_irom0_text_start;
    const char *end = &_irom0_text_end;

    return end - start;
}

int read_current_icache_seg0_size()
{
    uint32 readbuf[2];
    unsigned offset = 0;
    unsigned char nseg;

//    os_printf("Locating segments\n");

    const header_entry_t *hdr = (header_entry_t*)readbuf;
    spi_flash_read( offset, &readbuf[0], sizeof(header_entry_t));

    offset += sizeof(header_entry_t);

    if (hdr->magic != 0xe9) {
        return -1;
    }
    nseg = hdr->num_segments;
//    os_printf("Num segments %d\n", nseg);

    /* Read each segment header */
    while (nseg--) {
        // This should always be aligned.
        spi_flash_read( offset, &readbuf[0], sizeof(segment_entry_t));
        segment_entry_t *seg = (segment_entry_t*)readbuf;
        offset += sizeof(segment_entry_t);
//        os_printf("This segment %d\n", seg->size);
        offset += seg->size;
    }
    return (offset); // Size of next free
}

int get_free_flash()
{
    int total_flash = 512*1024; // 512KB
    int reserved_flash = 8*1024; // 8KB

    int irom0size = ALIGN(read_current_icache_seg0_size(), 4096);
    if (irom0size<0)
        return -1;
    int seg0size = ALIGN(read_current_irom0_size(), 4096);
    if (seg0size<0)
        return -1;

    return total_flash-(reserved_flash+irom0size+seg0size);
}

void clear_firmware_info(uint32_t val)
{
    uint32_t status = val;
    system_rtc_mem_write( RTC_FIRMWARE_OFFSET, &status, sizeof(status));
}

uint32_t get_last_firmware_status()
{
    uint32_t rtcoffset = RTC_FIRMWARE_OFFSET;
    firmware_info_t fwinfo;

    system_rtc_mem_read( rtcoffset, &fwinfo, sizeof(fwinfo));
    return fwinfo.magic;
}

int check_and_apply_firmware()
{
    firmware_info_t fwinfo;
    firmware_chunk_t chunk;
    uint8_t chunkno;
    uint8_t block;
    uint32_t rtcoffset = RTC_FIRMWARE_OFFSET;

    system_rtc_mem_read( rtcoffset, &fwinfo, sizeof(fwinfo));
    if ( fwinfo.magic != FIRMWARE_MAGIC )
        return 0; // No firmware

    /* Ok, we have a new firmware. */

    rtcoffset += sizeof(fwinfo)/sizeof(uint32_t);

    unsigned char *buf = os_malloc( SECTORSIZE );

    if (NULL==buf) {
        // Uups. Clear FW
        clear_firmware_info( FIRMWARE_ERROR_NORESOURCES );
        return -1;
    }

    for (chunkno=0; chunkno<fwinfo.chunks; chunkno++) {
        /* Read current chunk */
        system_rtc_mem_read( rtcoffset, &chunk, sizeof(chunk));
        rtcoffset += sizeof(chunk)/sizeof(uint32_t);
        for (block=0; block<chunk.chunksize_blocks; block++) {
            /* Read source block */
            uint32_t source = ((uint32_t)chunk.source_sector)<<SECTORBITS;
            if (spi_flash_read( source, (uint32*)buf, SECTORSIZE )<0) {
                clear_firmware_info( FIRMWARE_ERROR_SPIREADERROR | source );
                return -1;
            }
            /* Erase sector */
            if (spi_flash_erase_sector( (uint16)chunk.dest_sector) <0) {
                clear_firmware_info( FIRMWARE_ERROR_SPIERASEERROR | chunk.dest_sector );
                return -1;
            }
            /* Program sector */
            uint32_t dest = ((uint32_t)chunk.dest_sector)<<SECTORBITS;
            if (spi_flash_write( dest, (uint32*)buf, SECTORSIZE)<0) {
                clear_firmware_info( FIRMWARE_ERROR_SPIWRITEERROR | chunk.dest_sector );
                return -1;
            }
            chunk.source_sector++;
            chunk.dest_sector++;
        }
    }
    clear_firmware_info( FIRMWARE_UPGRADE_OK );
    system_restart();
    return 0;
}

void ota_initialize()
{
    /* Fill in chunks */
    int irom0size_sectors = ALIGN(read_current_icache_seg0_size(), SECTORSIZE) >> SECTORBITS;
    int seg0size_sectors =  ALIGN(read_current_irom0_size(), SECTORSIZE)>>SECTORBITS;

    ota_state.chunks[0].start_sector = CHUNK0_SECTOR_START + irom0size_sectors;
    ota_state.chunks[0].available = CHUNK0_SIZE - irom0size_sectors;
    ota_state.chunks[0].remaining = CHUNK0_SIZE - irom0size_sectors;
    ota_state.chunks[0].erased = 0;
    ota_state.chunks[0].block = 0;

    ota_state.chunks[1].start_sector = CHUNK1_SECTOR_START + seg0size_sectors;
    ota_state.chunks[1].available = CHUNK1_SIZE - seg0size_sectors;
    ota_state.chunks[1].remaining = CHUNK1_SIZE - seg0size_sectors;
    ota_state.chunks[1].block = 0;
    ota_state.chunks[1].erased = 0;

    ota_state.size = -1;

    ota_state.current_chunk = 0;
    ota_state.fwchunk = 0;

    // Debug only
    {
        int i;
        os_printf("OTA chunks: \n");
        for (i=0;i<OTA_NUM_CHUNKS;i++) {
            os_printf(" Chunk %d start sector 0x%02x, len %d sectors.\n",
                      i,
                      ota_state.chunks[i].start_sector,
                      ota_state.chunks[i].remaining);
        }
    }
}

static void ota_save_chunk()
{
    firmware_chunk_t c;
    struct ota_chunk *chunk = &ota_state.chunks[ ota_state.current_chunk ];

    unsigned offset = RTC_FIRMWARE_OFFSET +
        (sizeof(firmware_info_t)/sizeof(uint32));

    c.source_sector = chunk->start_sector;
    c.dest_sector = ota_state.start_address >> SECTORBITS;
    c.chunksize_blocks = chunk->available - chunk->remaining;

    os_printf("Saving chunk block, source sector %d, dest %d, size %d\n",
              c.source_sector,
              c.dest_sector,
              c.chunksize_blocks
             );


    offset += ota_state.fwchunk * (sizeof(firmware_chunk_t)/sizeof(uint32));

    system_rtc_mem_write( offset, &c, sizeof(c));

    ota_state.fwchunk++;

    // adjust chunk properties.
    chunk->start_sector = chunk->start_sector + c.chunksize_blocks;
    int leftover = chunk->available - c.chunksize_blocks;

    chunk->available = chunk->remaining = leftover;
    // and adjust source offset too
    ota_state.start_address += ((uint32)c.chunksize_blocks)<<SECTORBITS;
}

int ota_program_chunk(void *data) // Needs to be 512 bytes
{
    // See if we still have space in current chunk
    struct ota_chunk *chunk = &ota_state.chunks[ ota_state.current_chunk ];
    os_printf("Programming current chunk %d\n",ota_state.current_chunk);

    if (ota_state.size<BLOCKSIZE) {
        return -1;
    }

    if ( chunk->block == BLOCKS_PER_SECTOR ) {
        // Overflowed. Move to next sector.
        chunk->block = 0;
        chunk->erased = 0;
        chunk->remaining--;
        if (chunk->remaining) {
            // See if we still have room.

            os_printf("Remaining in this chunk: %d\n", chunk->remaining);
        } else {
            // Need to move to next chunk
            ota_save_chunk();
            ota_state.current_chunk++;
            if (ota_state.current_chunk == OTA_NUM_CHUNKS) {
                // No more room...
                return -1;
            }
            chunk = &ota_state.chunks[ ota_state.current_chunk ];
        }
    }

    // We are good to go. Get target sector and offset
    unsigned target_sector = chunk->start_sector + (chunk->available - chunk->remaining);

    unsigned target_offset = ((unsigned)target_sector * SECTORSIZE) +
        chunk->block * BLOCKSIZE;

    os_printf("Target sector is %d, offset 0x%x\n", target_sector, target_offset);

    // Do we need to erase ? If yes, do so
    if (!chunk->erased) {
        os_printf("Erasing sector %d\n", target_sector);
        if ( spi_flash_erase_sector( (uint16)target_sector ) < 0) {
            // Ups.
            return -1;
        }
        chunk->erased = 1;
    }
    // Write-out block
    os_printf("Writing block at 0x%8x\n", target_offset);
    if (spi_flash_write( target_offset, (uint32*)data, BLOCKSIZE)<0) {
        return -1;
    }
    // And increment it.
    chunk->block++;
    ota_state.size -= BLOCKSIZE;
    os_printf("Size to go: %d\n", ota_state.size);
    if (ota_state.size==0) {
        // Chunk done.
        ota_state.size=-1;
        chunk->block=0;
        chunk->remaining--;
        ota_save_chunk_block();
    }
    return 0;
}

void ota_save_chunk_block()
{
    ota_save_chunk();
}

int ota_finalize()
{
    firmware_chunk_t c;
    firmware_info_t fwinfo;

    os_printf("Finalizing OTA: chunks %d\n", ota_state.fwchunk);

    unsigned offset = RTC_FIRMWARE_OFFSET +
        (sizeof(firmware_info_t)/sizeof(uint32));

    int i;
    for (i=0;i<ota_state.fwchunk;i++) {
        system_rtc_mem_read( offset, &c, sizeof(c));

        offset += sizeof(firmware_chunk_t)/sizeof(uint32);

        os_printf(" > source sector %d, dest %d, size %d\n",
                  c.source_sector,
                  c.dest_sector,
                  c.chunksize_blocks
                 );

    }

    fwinfo.magic = FIRMWARE_MAGIC;
    fwinfo.chunks = ota_state.fwchunk;

    system_rtc_mem_write( RTC_FIRMWARE_OFFSET, &fwinfo, sizeof(fwinfo));

    system_restart();
}

int ota_set_chunk(uint32_t address, int size_bytes)
{
    if (ota_state.size==-1) {
        // Nothing set, we're OK.
        ota_state.start_address = address;
        if (((size_bytes & (SECTORSIZE-1)) !=0)) {
            os_printf("Attempting to set unaligned size of %d\n", size_bytes);
            return -1; // Need to sector aligned
        }
        ota_state.size = size_bytes;
        return 0;
    }
    return -1;
}



