#ifndef __OSTYPE_H__
#define __OSTYPE_H__

#ifdef HOST

#include <stdlib.h>
#include <inttypes.h>

typedef uint8_t uint8;
typedef int8_t sint8;

typedef uint16_t uint16;
typedef int16_t sint16;

typedef uint32_t uint32;

typedef void os_event_t;

#endif

#endif
