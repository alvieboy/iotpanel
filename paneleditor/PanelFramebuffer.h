#ifndef __PANELFRAMEBUFFER_H__
#define __PANELFRAMEBUFFER_H__

#include <QColor>
#include <inttypes.h>

class PanelFramebuffer
{
public:
    PanelFramebuffer( unsigned w, unsigned h);
    ~PanelFramebuffer();

    void set(unsigned x, unsigned y, const QColor &color) {
        if (x>=m_uWidth || x<0 || y>=m_uHeight || y<0)
            return;
        m_framebuffer[ (y*m_uWidth)+ x ] = color;
    }
    void set(unsigned x, unsigned y, uint8_t);

    const QColor &get(unsigned x, unsigned y) {
        return m_framebuffer[ (y*m_uWidth)+ x ];
    }

    void clear(const QColor &color) {
        unsigned int i;
        
        for (i=0;i< m_uWidth*m_uHeight;i++) {
            m_framebuffer[i] = color;
        }
    }
private:
    QColor *m_framebuffer;
    QColor test;
    unsigned m_uWidth, m_uHeight;
};

#endif
