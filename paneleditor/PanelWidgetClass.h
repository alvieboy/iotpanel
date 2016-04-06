#ifndef __PANELWIDGETCLASS_H__
#define __PANELWIDGETCLASS_H__

#include "PanelFramebuffer.h"
#include <QString>
#include <vector>
#include <QVariant>

typedef enum {
    T_INT,
    T_BOOL,
    T_STRING,
    T_COLOR
} ePropertyType;

struct PanelProperty
{
    PanelProperty(const QString &n) : name(n) {}
    PanelProperty() {}
    QString name;
    ePropertyType type;
    QVariant value;
};

class PanelWidgetClass
{
public:
    virtual void drawTo( PanelFramebuffer &dest, int x, int y) = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual const QString &getName() const = 0;
    virtual const QString &getClassName() const = 0;

    virtual const QList<PanelProperty*> &getProperties() const = 0;
};

#endif
