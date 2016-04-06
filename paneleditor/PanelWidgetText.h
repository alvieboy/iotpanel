#include "PanelWidgetClass.h"

class PanelWidgetText: public PanelWidgetClass
{
public:
    PanelWidgetText(const QString &name);
    virtual ~PanelWidgetText();
    virtual void drawTo( PanelFramebuffer &dest, int x, int y);
    virtual int getWidth() const { return -1; }
    virtual int getHeight() const { return 6; }
    virtual const QString &getName() const { return m_strName; }
    virtual const QString &getClassName() const { return m_className; }

    virtual void setText(const QString&s) { text=s; }
    virtual const QList<PanelProperty*> &getProperties() const { return m_properties; }

private:
    QString text;
    QString m_strName;
    static QString m_className;
    QList<PanelProperty*> m_properties;
};
