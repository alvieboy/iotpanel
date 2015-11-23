#ifndef __UPGRADE_H__
#define __UPGRADE_H__

#include "ets_sys.h"
#include "osapi.h"
#include "spi_flash.h"

#define FIRMWARE_MAGIC 0x1efa62db
#define FIRMWARE_ERROR_NORESOURCES 0xFF000000
#define FIRMWARE_ERROR_SPIREADERROR 0xFE000000
#define FIRMWARE_ERROR_SPIERASEERROR 0xFD000000
#define FIRMWARE_ERROR_SPIWRITEERROR 0xFC000000
#define FIRMWARE_UPGRADE_OK 0x1efa62ff

#define SECTORBITS 12
#define SECTORSIZE (1<<SECTORBITS)

#define RTC_FIRMWARE_OFFSET 64

typedef struct {
    uint32_t magic;
    uint8_t chunks;
    uint8_t pad;
    uint16_t crc;
} firmware_info_t;

typedef struct {
    uint8_t source_sector;
    uint8_t dest_sector;
    uint8_t chunksize_blocks;
    uint8_t pad;
} firmware_chunk_t;

int read_current_irom0_size();
int read_current_icache_seg0_size();
int get_free_flash();
int check_and_apply_firmware();
uint32_t get_last_firmware_status();
void ota_initialize();
int ota_set_chunk(uint32_t address, int size_bytes);
int ota_program_chunk(void *data); // Needs to be 512 bytes
int ota_finalize();


#define ALIGN(x, alignment) (((x)+(alignment-1)) & ~(alignment-1))

#endif
