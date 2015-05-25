#ifndef __GFX_H__
#define __GFX_H__

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

typedef struct gfxinfo
{
    int stride;
    int width;
    int height;
    uint8_t *fb;
} gfxinfo_t;

typedef struct 
{
    gfxinfo_t *gfx;
    const gfxinfo_t *dest;
    char str[128];
    int update;
    int x,y;
} scrollingtext_t;



static inline void drawPixel(const gfxinfo_t *gfx, int x, int y, uint8 color)
{
    x+=(y*gfx->stride);
    gfx->fb[x] = color;
}

void drawText(const gfxinfo_t*,int x, int y, const char *str, uint8 color, uint8 bg, uint8 size);
gfxinfo_t *allocateTextFramebuffer(const char *str);
gfxinfo_t *updateTextFramebuffer(gfxinfo_t *gfx, const char *str);

void setupScrollingText(scrollingtext_t *t, const gfxinfo_t *dest, int y, const char *str);
void updateScrollingText(scrollingtext_t *t, const char *str);
void drawScrollingText(scrollingtext_t *t);


#endif // __GFX_H__
