#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

LOCAL uint32 time_seconds;
LOCAL uint32 time_millis;

uint32 ICACHE_FLASH_ATTR time_get_seconds()
{
    return time_seconds;
}

void ICACHE_FLASH_ATTR time_get(uint32 *seconds, uint32 *millis)
{
    if (seconds)
        *seconds = time_seconds;
    if (millis)
        *millis = time_millis;
}


void ICACHE_FLASH_ATTR time_tick()
{
    time_millis++;
    if (time_millis==1000) {
        time_millis=0;
        time_seconds++;
    }
}

void ICACHE_FLASH_ATTR time_set(uint32 seconds, uint32 millis)
{
    time_seconds=seconds;
    time_millis=millis;
}
