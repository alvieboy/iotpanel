#ifndef __CRC_H__
#define __CRC_H__

#include "ets_sys.h"

uint16_t crc16_calc(uint8_t *data, unsigned size);
void crc16_update_buffer(uint16_t *crc, const uint8_t *d, unsigned size);

#endif