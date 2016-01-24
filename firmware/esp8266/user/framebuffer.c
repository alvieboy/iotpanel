#include "os_type.h"
#include "framebuffer.h"

volatile uint8_t bufferStatus[2];

framebuffer_t framebuffers[2];

void ICACHE_FLASH_ATTR init_framebuffers()
{
    bufferStatus[0] = BUFFER_FREE;
    bufferStatus[1] = BUFFER_DISPLAYING;
}

