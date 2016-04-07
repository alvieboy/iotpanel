#include "widget.h"
#include "ets_sys.h"
#include "osapi.h"
#include <string.h>

extern const widgetdef_t rectangle_widget;
extern const widgetdef_t scrollingtext_widget;
extern const widgetdef_t text_widget;
extern const widgetdef_t line_widget;
extern const widgetdef_t clock_widget;
extern const widgetdef_t chrono_widget;
extern const widgetdef_t romimage_widget;
extern const widgetdef_t gif_widget;

const widgetdef_t *widget_registry[] = {
    &rectangle_widget,
    &scrollingtext_widget,
    &text_widget,
    &line_widget,
    &clock_widget,
    &romimage_widget,
    &gif_widget,
//    &chrono_widget,
    0
};

const widgetdef_t *ICACHE_FLASH_ATTR widgetdef_find(const char *name)
{
    const widgetdef_t *w;
    int i;
    for ( i = 0; widget_registry[i]; i++) {
        w = widget_registry[i];
        if (strcmp(name,w->name)==0)
            return w;
    }
    return NULL;
}
