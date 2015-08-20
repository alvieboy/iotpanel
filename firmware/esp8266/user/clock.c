#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

LOCAL uint32 time_seconds;
LOCAL uint32 time_millis;
LOCAL uint32 lastclock = 0;
//LOCAL int offsetmillis = 0;
LOCAL int offsetseconds = 0;
LOCAL uint32 overflows = 0;

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

void ICACHE_FLASH_ATTR time_get_hmsm(uint8_t *hour, uint8_t *min, uint8_t *seconds, unsigned *millis)
{
    //printf("Sec %u\n",offsetseconds);
    uint32 t = time_seconds + offsetseconds;
    *millis = time_millis;
    *seconds = t%60;
    t = t/60;
    *min = t%60;
    t = t/60;
    *hour = t % 24;
}


void ICACHE_FLASH_ATTR time_tick()
{
#if 1
#else
    time_millis++;
    if (time_millis==1000) {
        time_millis=0;
        time_seconds++;
    }
#endif
    uint32 clkmillis = system_get_time()/1000;

    if (clkmillis < lastclock) {
        /* Overflow */
        overflows++;
    }
    lastclock = clkmillis;

    // 4294967.3
    // 4294.9673
    time_seconds = (clkmillis/1000) + (overflows * 4249);
    time_millis = (clkmillis%1000) + (overflows * 967);
    if (time_millis>1000) {
        time_seconds += time_millis/1000;
        time_millis = time_millis%1000;
    }
}

void ICACHE_FLASH_ATTR time_set(uint32 seconds, uint32 millis)
{
    time_seconds=seconds;
    time_millis=millis;
}

void ICACHE_FLASH_ATTR time_set_seconds(uint32 seconds)
{
    offsetseconds = (int)seconds - (int)time_seconds;
}
