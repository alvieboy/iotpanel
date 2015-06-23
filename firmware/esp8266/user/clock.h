#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

uint32 time_get_seconds();
void time_get(uint32 *seconds, uint32 *millis);
void time_tick();
void time_set(uint32 seconds, uint32 millis);

#endif
