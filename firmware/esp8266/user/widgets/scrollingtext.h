#ifndef __SCROLLINGTEXT_H__
#define __SCROLLINGTEXT_H__

#include "gfx.h"
#include "color.h"

#define SCROLLINGTEXT(w) ((scrollingtext_t*)((w)->priv))


typedef struct
{
    gfxinfo_t *gfx;
    const gfxinfo_t *dest;
    char str[128];
    int update;
    int x,y;
    uint8 fg,bg;
    const font_t *font;
} scrollingtext_t;

void setupScrollingText(scrollingtext_t *t, const gfxinfo_t *dest, const font_t*,int y, const char *str);
void updateScrollingText(scrollingtext_t *t, const char *str);
void drawScrollingText(scrollingtext_t *t);

#endif
