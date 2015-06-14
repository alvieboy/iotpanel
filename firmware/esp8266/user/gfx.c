#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "gfx.h"
#include "mem.h"
#include "alloc.h"

void drawChar(const gfxinfo_t *gfx, const font_t *font, int x, int y, unsigned char c,
              uint8 color, uint8 bg)
{
    const uint8 *cptr;
    uint8 hc = font->h;

    if ( (c<font->start) || (c>font->end)) {
        c = font->start;
    }
    cptr = &font->bitmap[(c - font->start)*(font->h)];

    // Draw.
    do {
        uint8 line = *cptr++;
        uint8 wc = font->w;
        int sx=x;
        do {
            int pixel = line & 0x80;
            line <<=1;
            if (pixel) {
                drawPixel(gfx, x, y, color);
            } else if (bg != color) {
                drawPixel(gfx, x, y, bg);
            }
            x++;
        } while (--wc);
        x=sx;
        y++;
    } while (--hc);
}

void ICACHE_FLASH_ATTR drawText(const gfxinfo_t *gfx, const font_t *font, int x, int y, const char *str, uint8 color, uint8 bg)
{
    while (*str) {
        drawChar(gfx, font,x,y,*str,color,bg);
        x+=font->w;
        str++;
    }
}

LOCAL void ICACHE_FLASH_ATTR freeTextFramebuffer(gfxinfo_t *info)
{
    if (info->fb)
        os_free(info->fb);
    os_free(info);
}

gfxinfo_t * ICACHE_FLASH_ATTR allocateTextFramebuffer(const char *str, const font_t *font)
{
    int size = strlen(str);
    gfxinfo_t *info = os_malloc(sizeof(gfxinfo_t));
    if (info==NULL)
        return NULL;
    size *= font->w;
    info->width  = size;
    info->stride = size;
    info->height = font->h;
    /**/
    size *= font->h;
    info->fb = os_malloc(size);
    memset(info->fb,0,size);
    return info;
}

gfxinfo_t * ICACHE_FLASH_ATTR updateTextFramebuffer(gfxinfo_t *gfx, const font_t *font, const char *str)
{
    int size = strlen(str) * font->w;
    if (size > gfx->stride) {
        freeTextFramebuffer(gfx);
        gfx = allocateTextFramebuffer(str,font);
    } else {
        gfx->width = size;
        memset(gfx->fb,0,gfx->height * gfx->stride);
    }
    return gfx;
}

int ICACHE_FLASH_ATTR overlayFramebuffer( const gfxinfo_t *source, const gfxinfo_t *dest, int x, int y)
{
    int ptr;
    int src;

    ptr = (y*dest->stride);
    // Clip
    if (x > dest->width)
        return 1;
    int w;

    if (x>0) {
        ptr += x;
        src = 0;
        w = dest->width - x;
    } else {
        src = -x;
        w = dest->width;
    }

    if (w > (source->width-src)) {
        w = source->width-src;
    }

    if (w<=0) {
        if (x>0) {
            return 1;
        }
        return -1;
    }

    /* draw */
    int cw,ch;

    for (ch=0; ch<source->height; ch++) {
        for (cw=0; cw<w; cw++) {
            dest->fb[ptr + cw] = source->fb[src + cw];
        }
        ptr+=dest->stride;
        src+=source->stride;
    }

    return 0;
}

void ICACHE_FLASH_ATTR gfx_clear(gfxinfo_t *gfx)
{
    memset(gfx->fb, 0,gfx->height * gfx->stride);
}




