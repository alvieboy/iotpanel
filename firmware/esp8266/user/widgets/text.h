#ifndef __TEXT_H__
#define __TEXT_H__

#include "gfx.h"
#include "color.h"

#define TEXT(w) ((text_t*)((w)->priv))


typedef struct
{
    gfxinfo_t *gfx;
    const gfxinfo_t *dest;
    char str[128];
    int update;
    uint8 fg,bg;
    const font_t *font;
} text_t;

void setupText(text_t *t, const gfxinfo_t *dest, const font_t*, const char *str);
void updateText(text_t *t, const char *str);
void drawTextWidget(text_t *t, int x, int y);

#endif
