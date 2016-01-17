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
#include <ctype.h>

#ifndef swap
#define swap(a, b) { int t = a; a = b; b = t; }
#endif

#define ESCAPE '~'

LOCAL int ICACHE_FLASH_ATTR textComputeLength(const char *str, const textrendersettings_t *s, int *width, int *height);
LOCAL int getNumberOfPrintableChars(const char *str, const textrendersettings_t *settings);

LOCAL int unpackHexByte(const char *str, uint8_t *dest);

LOCAL void ICACHE_FLASH_ATTR drawChar16(const gfxinfo_t *gfx, const font_t *font, int x, int y, unsigned char c,
                                        uint8 color, uint8 bg)
{
    const uint8 *cptr;
    //printf("Draw 16 %d %d \n", font->w, font->h);
    uint8 hc = font->h;
    uint16 mask = 0x8000;//(1<<(font->w-1));

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
LOCAL int ICACHE_FLASH_ATTR parseUnprintable(const char**str, uint8 *color, uint8 *bg)
{
    uint8_t code;

    if (**str==ESCAPE) {
        (*str)++;
        if (**str=='\0')
            return 0;
        switch (**str) {
        case 'f':
            (*str)++;
            if (unpackHexByte(*str, &code)<0)
                return -1;
            *color = code;
            (*str)+=2;
            break;
        case 'b':
            (*str)++;
            if (unpackHexByte(*str, &code)<0)
                return -1;
            *bg = code;
            (*str)+=2;
            break;
        case 'c':
            (*str)++;
            if (unpackHexByte(*str, &code)<0)
                return -1;
            *bg = *color = code;
            (*str)+=2;
            break;
        default:
            return 0;
        }
    }

    return 0;
}


#if 1
void ICACHE_FLASH_ATTR drawText(const gfxinfo_t *gfx, const textrendersettings_t *s, int x, int y, const char *str, uint8 color, uint8 bg)
{
    int i,j;
    int sx = x;
    DEBUG("Draw text bg %d fg %d, maxwidth %d, wrap %d\n", (int)bg, (int)color, (int)s->w, (int)s->wrap);
    do {
        i = getNumberOfPrintableChars(str, s);
        if (i<0) {
            return;
        }
        x = sx;
#if 1
        if (s->w >0) {

            if (s->align == ALIGN_RIGHT ) {
                // Check excess

                int used = (s->font->w * i);
                //printf("Align right: used %d, avail %d\n", used, gfx->width);
                used = gfx->width - used;
                //if (used<0)
                //    used=0;
                x+=used;
                //printf("Offset now %d fixup %d\n",x,used );
            }
        }
#endif
        DEBUG("Printing %d chars at %d %d\n", i, x, y);
        for (j=0;j<i;j++) {

            if (parseUnprintable(&str,&color,&bg)<0)
                return;
            drawChar(gfx, s->font,x,y,*str,color,bg);
            x += s->font->w;
            str++;
        }
        y += s->font->h+1;

    } while (*str);
}
#else
void ICACHE_FLASH_ATTR drawText(const gfxinfo_t *gfx, const textrendersettings_t *s, int x, int y, const char *str, uint8 color, uint8 bg)
{
    DEBUG("Draw text bg %d fg %d\n", bg, color);
    uint8_t code;

    while (*str) {
        if (*str==ESCAPE) {
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

        drawChar(gfx, s->font,x,y,*str,color,bg);
        x+=s->font->w;
        str++;
    }
}
#endif

LOCAL void ICACHE_FLASH_ATTR freeTextFramebuffer(gfxinfo_t *info)
{
    if (info->fb) {
        DEBUG("Freeing fb @ %p\n", info->fb);
        os_free(info->fb);
        info->fb = NULL;
    }
    DEBUG("Freeing info @ %p\n", info);
    os_free(info);
}

gfxinfo_t * ICACHE_FLASH_ATTR allocateTextFramebuffer(const char *str, const textrendersettings_t *s)
{
    int size_x, size_y;
    if (textComputeLength(str,s,&size_x,&size_y)<0)
        return NULL;

    gfxinfo_t *info = os_calloc(sizeof(gfxinfo_t),1);
    DEBUG("New info @ %p\n", info);

    if ((info==NULL) || (s->font==NULL))
        return NULL;

    //size_x *= s->font->w;
    info->width  = size_x;
    info->stride = size_x;
    info->height = size_y;
    /**/
    int size = size_x * size_y;
    DEBUG("New fb size %d x %d: %d '%s'\n", size_x, size_y, size, str);

    if (size==0) {
        info->fb = NULL;
    } else {
        info->fb = os_calloc(size,1);
    }
    DEBUG("New fb: %p\n",info->fb);
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

LOCAL int ICACHE_FLASH_ATTR skipUnprintable(const char **str)
{
    uint8_t code;
    if (*(*str)==ESCAPE) {
        (*str)++;
        if (*(*str)=='\0')
            return -1;
        switch (*(*str)) {
        case 'f':
        case 'b':
        case 'c':
            (*str)++;
            if (unpackHexByte((*str), &code)<0)
                return -1;
            (*str)+=2;
            break;
        default:
            return -1;
        }
    }
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR getNumberOfPrintableChars(const char *str,
                                    const textrendersettings_t *settings)
{
    int skip;
    int count = 0;
    const char *lastspace = NULL;
    int maxwidth = settings->w;
    /* Skip spaces first? */

    do {
        skip = skipUnprintable(&str);
        if (skip<0) {
            return -1;
        }
        if (maxwidth>=0) {
            if (maxwidth < settings->font->w) {

                if (settings->wrap && lastspace) {
                    //printf("lastspace\n");
                    lastspace++;
                    //printf("will restart at '%s'\n",lastspace);
                    count-=(str-lastspace);
                }
                return count;
            }
            if (isspace(*str))
                lastspace = str;
        }
        count++;
        str++;
        if (maxwidth>0) {
            maxwidth-=settings->font->w;
        }
    } while (*str);

    return count;
}


LOCAL int ICACHE_FLASH_ATTR textComputeLength(const char *str, const textrendersettings_t *s, int *width, int *height)
{
    int i;
    int maxw = 0;
    int maxh = 0;
    do {
        DEBUG("Str: '%s'\n", str);
        i = getNumberOfPrintableChars(str, s);
        DEBUG("Printable chars: %d\n", i);
        if (i<0) {
            return -1;
        }
        maxh++;
        str+=i;
        DEBUG("Str now: '%s'\n", str);
        if (maxw<i)
            maxw=i;
    } while (*str);
    *width = (maxw * s->font->w);// + (maxw-1);  // Spacing between chars
    *height = (maxh * s->font->h) + (maxh-1); // Spacing between chars
    return 0;
}

gfxinfo_t * ICACHE_FLASH_ATTR updateTextFramebuffer(gfxinfo_t *gfx, const textrendersettings_t *s, const char *str)
{
    //    int size = strlen(str) * font->w;
    //if (size > gfx->stride) {
    DEBUG("Freeing fb\n");
    freeTextFramebuffer(gfx);
    DEBUG("Alloc fb for '%s'\n", str);
    gfx = allocateTextFramebuffer(str,s);
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

void ICACHE_FLASH_ATTR destroyTextFramebuffer(gfxinfo_t *info)
{
    if (info) {
        if (info->fb)
            os_free(info->fb);
        os_free(info);
    }
}


