#include "crc.h"

#define DEBUGCRC(x...)

static inline void crc16_update(uint16_t *crc, uint8_t data)
{
    data ^= (*crc)&0xff;
    data ^= data << 4;
    (*crc) = ((((uint16_t)data << 8) | (((*crc)>>8)&0xff)) ^ (uint8_t)(data >> 4)
              ^ ((uint16_t)data << 3));
}

void ICACHE_FLASH_ATTR crc16_update_buffer(uint16_t *crc, const uint8_t *d, unsigned size)
{
    uint8_t data;
    while (size--) {
        data = *d++;
        data ^= (*crc)&0xff;
        data ^= data << 4;
        (*crc) = ((((uint16_t)data << 8) | (((*crc)>>8)&0xff)) ^ (uint8_t)(data >> 4)
                  ^ ((uint16_t)data << 3));
    }
}

uint16_t ICACHE_FLASH_ATTR crc16_calc(uint8_t *data, unsigned size)
{
    uint16_t crc = 0;
    crc16_update_buffer(&crc, data, size);
    DEBUGCRC("Final: %04x\n", crc);
    return crc;
}


