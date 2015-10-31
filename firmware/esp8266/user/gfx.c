#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "gfx.h"
//#include "mem.h"
#include "alloc.h"
#include <string.h>
#include "debug.h"
#include "protos.h"
#include "color.h"

#ifndef swap
#define swap(a, b) { int t = a; a = b; b = t; }
#endif

LOCAL int ICACHE_FLASH_ATTR textComputeLength(const char *str);
LOCAL int unpackHexByte(const char *str, uint8_t *dest);

LOCAL void ICACHE_FLASH_ATTR drawChar16(const gfxinfo_t *gfx, const font_t *font, int x, int y, unsigned char c,
                                        uint8 color, uint8 bg)
{
    const uint8 *cptr;
    //printf("Draw 16 %d %d \n", font->w, font->h);
    uint8 hc = font->h;
    uint16 mask = (1<<(font->w-1));

    if ( (c<font->start) || (c>font->end)) {
        c = font->start;
    }
    cptr = (uint8*) ( &font->bitmap[(c - font->start)*(font->h)*2] );
    //printf("Start char %d, index is %d\n", c,  (c - font->start)*(font->h)*2 );
    // Draw.
    do {
        uint16 line = (uint16)(*cptr++)<<8;
        line += (uint16)*cptr++;
        //printf("Line: 0x%04x mask 0x%04x\n",line,mask);
        uint8 wc = font->w;
        int sx=x;
        do {
            int pixel = line & mask;
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


void ICACHE_FLASH_ATTR drawChar(const gfxinfo_t *gfx, const font_t *font, int x, int y, unsigned char c,
              uint8 color, uint8 bg)
{
    const uint8 *cptr;

    if ((NULL==font) || (NULL==gfx))
        return;

    if (font->w > 8) {
        drawChar16(gfx, font, x, y, c, color, bg);
        return;
    }

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
    DEBUG("Draw text bg %d fg %d\n", bg, color);
    uint8_t code;

    while (*str) {
        if (*str==0x1b) {
            str++;
            if (*str=='\0')
                return;
            switch (*str) {
            case 'f':
                if (unpackHexByte(++str, &code)<0)
                    return;
                color = code;
                str+=2;
                break;
            case 'b':
                if (unpackHexByte(++str, &code)<0)
                    return;
                bg = code;
                str+=2;
                break;
            case 'c':
                if (unpackHexByte(++str, &code)<0)
                    return;
                bg = color = code;
                str+=2;
                break;
            default:
                return;
            }
            continue;
        }

        drawChar(gfx, font,x,y,*str,color,bg);
        x+=font->w;
        str++;
    }
}

LOCAL void ICACHE_FLASH_ATTR freeTextFramebuffer(gfxinfo_t *info)
{
    if (info->fb) {
        DEBUG("Freeing fb @ %p\n", info->fb);
        os_free(info->fb);
    }
    DEBUG("Freeing info @ %p\n", info);
    os_free(info);
}

gfxinfo_t * ICACHE_FLASH_ATTR allocateTextFramebuffer(const char *str, const font_t *font)
{
    int size = textComputeLength(str);
    if (size==-1)
        return NULL;

    gfxinfo_t *info = os_calloc(sizeof(gfxinfo_t),1);
    DEBUG("New info @ %p\n", info);

    if ((info==NULL) || (font==NULL))
        return NULL;

    size *= font->w;
    info->width  = size;
    info->stride = size;
    info->height = font->h;
    /**/
    size *= font->h;
    DEBUG("New fb size: %d '%s'\n",size,str);

    if (size==0) {
        info->fb = NULL;
    } else {
        info->fb = os_calloc(size,1);
    }
    DEBUG("New info: %p\n",info);
    return info;
}

LOCAL int ICACHE_FLASH_ATTR unpackHexNibbleOr(const char *str, uint8_t *dest)
{
    
    char l = *str;
    if (l>='0' && l<='9') {
        *dest |= (l-'0');
        return 0;
    }

    if (l>='a' && l<='z') {
        *dest |= (l-'a'+10);
        return 0;
    }
    return -1;
}

LOCAL int unpackHexByte(const char *str, uint8_t *dest)
{
    *dest = 0;
    if (unpackHexNibbleOr(str,dest)<0)
        return -1;
    *dest<<=4, str++;
    if (unpackHexNibbleOr(str,dest)<0)
        return -1;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR textComputeLength(const char *str)
{
    int size = 0;
    uint8_t code;
    /* We need to skip color stuff */
    while (*str) {
        if (*str==0x1b) {
            str++;
            if (*str=='\0')
                return -1;
            switch (*str) {
            case 'f':
            case 'b':
            case 'c':
                if (unpackHexByte(++str, &code)<0)
                    return -1;
                str+=2;
                break;
            default:
                return -1;
            }
            continue;
        }
        str++, size++;
    }
    return size;
}

gfxinfo_t * ICACHE_FLASH_ATTR updateTextFramebuffer(gfxinfo_t *gfx, const font_t *font, const char *str)
{
    //    int size = strlen(str) * font->w;
    //if (size > gfx->stride) {
    DEBUG("Freeing fb\n");
    freeTextFramebuffer(gfx);
    DEBUG("Alloc fb for '%s'\n", str);
    gfx = allocateTextFramebuffer(str,font);
#if 0
    } else {
        gfx->width = size;
        memset(gfx->fb,0,gfx->height * gfx->stride);
    }
#endif
    return gfx;
}

int ICACHE_FLASH_ATTR overlayFramebuffer( const gfxinfo_t *source, const gfxinfo_t *dest, int x, int y, int transparent)
{
    int ptr;
    int src;

    if (source==NULL)
        return 1;

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
    y = dest->height - y;
    //DEBUG("Source height: %d\n", source->height);
    for (ch=0; ch<source->height; ch++) {

        if (y<=0)
            break;

        for (cw=0; cw<w; cw++) {
            if (transparent<0 || (transparent != source->fb[src + cw]))
                dest->fb[ptr + cw] = source->fb[src + cw];
        }
        ptr+=dest->stride;
        src+=source->stride;
        y--;
    }

    return 0;
}

void ICACHE_FLASH_ATTR gfx_clear(gfxinfo_t *gfx)
{
    memset(gfx->fb, 0,gfx->height * gfx->stride);
}



void ICACHE_FLASH_ATTR gfx_drawLine(gfxinfo_t *gfx,
                                    int x0, int y0,
                                    int x1, int y1,
                                    color_t color) {
  int steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int err = dx / 2;
  int ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(gfx, y0, x0, color);
    } else {
      drawPixel(gfx, x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}




