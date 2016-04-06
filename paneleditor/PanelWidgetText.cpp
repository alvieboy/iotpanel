#include "PanelWidgetText.h"
#include "font.h"
#include "text.h"


PanelWidgetText::PanelWidgetText(const QString &name): m_strName(name)
{
    m_properties.append( new PanelProperty("teste") );
    m_properties.append( new PanelProperty("teste2") );
}

PanelWidgetText::~PanelWidgetText()
{
    foreach (PanelProperty *i, m_properties)
        delete(i);
}

void PanelWidgetText::drawTo( PanelFramebuffer &dest, int x, int y)
{
    const font_t *thumb = font_find("thumb");
    PanelText t;
    t.drawText(dest, thumb, x, y, text.toStdString().c_str(), 0xff, 0xff);
}

QString PanelWidgetText::m_className("text");
