#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "gfx.h"
#include "mem.h"

void *pvPortMalloc( size_t xWantedSize );

extern const unsigned char font[];

void drawChar(const gfxinfo_t *gfx, int x, int y, unsigned char c,
              uint8 color, uint8 bg, uint8 size)
{
    uint8 i,j;
    if((x >= gfx->width)            || // Clip right
       (y >= gfx->height)           || // Clip bottom
       ((x + 6 * size - 1) < 0) || // Clip left
       ((y + 8 * size - 1) < 0))   // Clip top
        return;

    for (i=0; i<6; i++ ) {
        uint8 line;
        if (i == 5)
            line = 0x0;
        else
            line = font[(c*5)+i];
        for (j = 0; j<8; j++) {
            if (line & 0x1) {
                if (size == 1) // default size
                    drawPixel(gfx,x+i, y+j, color);
                else {  // big size
                    //fillRect(x+(i*size), y+(j*size), size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) // default size
                    drawPixel(gfx,x+i, y+j, bg);
                else {  // big size
                    //fillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
}

void drawText(const gfxinfo_t *gfx, int x, int y, const char *str, uint8 color, uint8 bg, uint8 size)
{
    while (*str) {
        drawChar(gfx,x,y,*str,color,bg,size);
        x+=6; str++;
    }
}

static void freeTextFramebuffer(gfxinfo_t *info)
{
    if (info->fb)
        os_free(info->fb);
    os_free(info);
}

gfxinfo_t *allocateTextFramebuffer(const char *str)
{
    int size = strlen(str);
    gfxinfo_t *info = os_malloc(sizeof(gfxinfo_t));
    if (info==NULL)
        return NULL;
    size *= 6;
    info->width  = size;
    info->stride = size;
    info->height = 7;
    size*=7;
    info->fb = os_malloc(size);
    memset(info->fb,0,size);
    return info;
}

gfxinfo_t *updateTextFramebuffer(gfxinfo_t *gfx, const char *str)
{
    int size = strlen(str) * 6;
    if (size > gfx->stride) {
        freeTextFramebuffer(gfx);
        gfx = allocateTextFramebuffer(str);
    } else {
        gfx->width = size;
        memset(gfx->fb,0,gfx->height * gfx->stride);
    }
    return gfx;
}

int overlayFramebuffer( const gfxinfo_t *source,  const gfxinfo_t *dest, int x, int y)
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

void clearFramebuffer(gfxinfo_t *gfx)
{
    memset(gfx->fb, 0,gfx->height * gfx->stride);
}

void setupScrollingText(scrollingtext_t *t, const gfxinfo_t *dest, int y, const char *str)
{
    if (t==NULL)
        return;

    t->dest = dest;
    t->x = dest->width-1;
    t->y = y;
    updateScrollingText(t, str);

    t->gfx = allocateTextFramebuffer(str);
    drawText( t->gfx, 0,0, str,  0x7, 0x7, 1);

}

void updateScrollingText(scrollingtext_t *t, const char *str)
{
    if (t==NULL)
        return;

    strcpy(t->str, str);
    t->update = 1;
}

void drawScrollingText(scrollingtext_t *t)
{
    if (t==NULL)
        return;
    switch (overlayFramebuffer(t->gfx, t->dest, t->x, t->y)) {
    case -1:
        t->x = t->dest->width-1;
        if (t->update) {
            if (t->gfx) {
                updateTextFramebuffer(t->gfx, t->str);
            } else {
                t->gfx = allocateTextFramebuffer(t->str);
            }
            /* Draw */
            drawText( t->gfx, 0,0, t->str,  0x5, 0x5, 1);
            t->update = 0;
        }

        break;
    default:
        t->x--;
        break;
    }
}



