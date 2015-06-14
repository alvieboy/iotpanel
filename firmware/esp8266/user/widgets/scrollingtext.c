#include "scrollingtext.h"
#include "widget.h"
#include "alloc.h"

void ICACHE_FLASH_ATTR setupScrollingText(scrollingtext_t *t, const gfxinfo_t *dest,
                                           const font_t *font,
                                           int y, const char *str)
{
    if (t==NULL)
        return;

    t->dest = dest;
    t->x = dest->width-1;
    t->y = y;
    t->font = font;
    t->fg = 0x3;
    t->bg = 0x3;
    updateScrollingText(t, str);

    t->gfx = allocateTextFramebuffer(str, t->font);
    drawText( t->gfx, t->font,0,0, str,  t->fg, t->bg);

}

void ICACHE_FLASH_ATTR updateScrollingText(scrollingtext_t *t, const char *str)
{
    if (t==NULL)
        return;

    strcpy(t->str, str);
    t->update = 1;
}

void ICACHE_FLASH_ATTR drawScrollingText(scrollingtext_t *t)
{
    if (t==NULL)
        return;
    switch (overlayFramebuffer(t->gfx, t->dest, t->x, t->y)) {
    case -1:
        t->x = t->dest->width-1;
        if (t->update) {
            if (t->gfx) {
                updateTextFramebuffer(t->gfx, t->font, t->str);
            } else {
                t->gfx = allocateTextFramebuffer(t->str, t->font);
            }
            /* Draw */
            drawText( t->gfx, t->font, 0,0, t->str,  t->fg, t->bg);
            t->update = 0;
        }

        break;
    default:
        t->x--;
        break;
    }
}

int ICACHE_FLASH_ATTR scrollingtext_set_font(widget_t *w, char *name)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    t->font = font_find(name);
    if (t->font==NULL)
        return -1;
    return 0;
}

int ICACHE_FLASH_ATTR scrollingtext_set_color(widget_t *w, char *name)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    if (color_parse(name, &t->fg)<0)
        return -1;
    t->bg = t->fg;
    return 0;
}

static property_t properties[] = {
    { "text",  T_STRING, SETTER(updateScrollingText),  NULL },
    { "font",  T_STRING, SETTER(scrollingtext_set_font),  NULL },
    { "color",  T_STRING, SETTER(scrollingtext_set_color),  NULL },
    END_OF_PROPERTIES
};

static widget_t *ICACHE_FLASH_ATTR scrollingtext_new(void*what)
{
    return os_malloc(sizeof(scrollingtext_t));
}

static void ICACHE_FLASH_ATTR scrollingtext_destroy(void*what)
{
    os_free(what);
}

void ICACHE_FLASH_ATTR scrollingtext_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    t->y = y;

    drawScrollingText(t);
}

widgetdef_t scrollingtext_widget = {
    .name = "scrollingtext",
    .properties = properties,
    .alloc = &scrollingtext_new,
    .redraw = &scrollingtext_redraw,
    .destroy = &scrollingtext_destroy
};
