#include "PanelFramebuffer.h"

PanelFramebuffer::PanelFramebuffer( unsigned w, unsigned h): m_uWidth(w), m_uHeight(h)
{
    m_framebuffer = new QColor[ w*h ];
}
PanelFramebuffer::~PanelFramebuffer()
{
    delete[] m_framebuffer;
}


void PanelFramebuffer::set(unsigned x, unsigned y, uint8_t v)
{
    int r,g,b;
    r = (v&1) * 255;
    g = ((v&2)>>1) * 255;
    b = ((v&4)>>2) * 255;
    set(x,y,QColor(r,g,b));
}
