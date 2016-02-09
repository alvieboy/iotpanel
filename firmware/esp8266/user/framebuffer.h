#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <os_type.h>

#define HORIZONTAL_PANELS 2

#define FRAMEBUFFER_SIZE (32*32*HORIZONTAL_PANELS)

typedef uint8_t pixel_t;

typedef pixel_t framebuffer_t[FRAMEBUFFER_SIZE];

extern framebuffer_t framebuffers[2];

extern volatile uint8_t bufferStatus[2];

#define BUFFER_FREE 0
#define BUFFER_READY 1
#define BUFFER_DISPLAYING 2

void init_framebuffers();

#endif
