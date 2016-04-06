#ifndef __PANELITEM_H__
#define __PANELITEM_H__

#include "PanelWidgetClass.h"
#include "PanelFramebuffer.h"
#include <QRect>

class PanelItem
{
public:
    PanelItem( PanelWidgetClass *itemclass, int x, int y ): m_class(itemclass), m_x(x), m_y(y)
    {
        m_grab=false;
        m_selected=false;
    }

    QRect getScreenRect(unsigned maxwidth_leds, unsigned maxheight_leds) const;

    void drawTo(PanelFramebuffer &dest) {
        m_class->drawTo(dest,m_x,m_y);
    }
    const PanelWidgetClass *getClass() const { return m_class; }

    int getX() const { return m_x; }
    int getY() const { return m_y; }
    void move(int x, int y) { m_x=x; m_y=y; }
    void grab() { m_grab=true; m_selected=true; }
    void select() { m_selected=true; }
    void deselect() { m_selected=false; m_grab=false; }
    void release() { m_grab = false; }
    bool isGrabbed() const { return m_grab; }
    bool isSelected() const { return m_selected; }

private:
    PanelWidgetClass *m_class;
    int m_x,m_y;

    bool m_grab;
    bool m_selected;
};

#endif
