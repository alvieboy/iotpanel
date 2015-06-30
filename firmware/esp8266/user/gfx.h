#ifndef __GFX_H__
#define __GFX_H__

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "font.h"

typedef struct gfxinfo
{
    int stride;
    int width;
    int height;
    uint8_t *fb;
} gfxinfo_t;


void gfx_clear(gfxinfo_t *gfx);

static inline void drawPixel(const gfxinfo_t *gfx, int x, int y, uint8 color)
{
    x+=(y*gfx->stride);
    gfx->fb[x] = color;
}

void drawText(const gfxinfo_t*,const font_t*,int x, int y, const char *str, uint8 color, uint8 bg);
gfxinfo_t *allocateTextFramebuffer(const char *str, const font_t*);
gfxinfo_t *updateTextFramebuffer(gfxinfo_t *gfx, const font_t*,const char *str);
int overlayFramebuffer( const gfxinfo_t *source, const gfxinfo_t *dest, int x, int y, int transparentcolor);

#endif // __GFX_H__
