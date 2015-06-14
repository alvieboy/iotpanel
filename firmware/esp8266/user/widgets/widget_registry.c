#include "widget.h"
#include "ets_sys.h"
#include "osapi.h"

extern const widgetdef_t rectangle_widget;
extern const widgetdef_t scrollingtext_widget;

const widgetdef_t *widget_registry[] = {
    &rectangle_widget,
    &scrollingtext_widget,
    0
};

const widgetdef_t *widgetdef_find(const char *name)
{
    const widgetdef_t *w;
    for (w = widget_registry[0]; w; w++) {
        if (strcmp(name,w->name)==0)
            return w;
    }
    return NULL;
}
