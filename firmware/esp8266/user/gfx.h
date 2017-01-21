#ifndef __GFX_H__
#define __GFX_H__


#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "font.h"
#include "color.h"
#include "framebuffer.h"

typedef struct gfxinfo
{
    int stride;
    int width;
    int height;
    pixel_t *fb;
} gfxinfo_t;

typedef enum {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER
} alignment_t;

typedef struct {
    const font_t *font;
    int8_t w; // width clip
    int8_t h; // height clip
    alignment_t align;
    uint8_t wrap:1; // Wrap or not text
    uint8_t rotate:1; // Rotate
} textrendersettings_t;

void gfx_clear(gfxinfo_t *gfx);

static inline void drawPixel(const gfxinfo_t *gfx, int x, int y, uint8 color)
{
    x+=(y*gfx->stride);
    gfx->fb[x] = color;
}

void drawText(const gfxinfo_t*,const textrendersettings_t*,int x, int y, const char *str, uint8 color, uint8 bg);
gfxinfo_t *allocateTextFramebuffer(const char *str, const textrendersettings_t*);
gfxinfo_t *updateTextFramebuffer(gfxinfo_t *gfx, const textrendersettings_t*,const char *str);
int overlayFramebuffer( const gfxinfo_t *source, const gfxinfo_t *dest, int x, int y, int transparentcolor);
void destroyTextFramebuffer(gfxinfo_t *fb);

void gfx_drawLine(gfxinfo_t *gfx,
                  int x0, int y0,
                  int x1, int y1,
                  color_t color);

#endif // __GFX_H__
