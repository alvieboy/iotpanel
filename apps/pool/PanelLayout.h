#ifndef __PANELLAYOUT_H__
#define __PANELLAYOUT_H__

#include <QString>
#include <QPoint>
#include <QList>
#include <QFile>
#include <QMap>
#include <QVariant>
#include <QDomNode>
#include <QSharedDataPointer>

class QDomDocument;

class LayoutItemInstance: public QSharedData
{
public:
    QString name;
    QString description;
    QString classname;
    QMap<QString, QString> properties;
};

typedef QSharedDataPointer<LayoutItemInstance> LayoutItemInstancePtr;

class LayoutItemEntry
{
public:
    LayoutItemInstancePtr instance;
    QPoint pos;
};

class LayoutScreen
{
public:
    QString name;
    QString description;
    QList<LayoutItemEntry> items;
};

class LayoutScheduleEntry
{
    enum { SELECT, WAIT } command;
    QVariant arg;
};

class LayoutSchedule
{
    QString name;
    QString description;
    QList<LayoutScheduleEntry> commands;
};

class PanelLayout
{
public:
    int createFromFile(QFile &);
    int writeToFile(QFile &);
    int serialize(QStringList &);
    int process(QDomDocument &doc);
    int processScreen(QDomElement );
    int processScreenItem(LayoutScreen &screen, QDomElement e);
    int processItemProperties(LayoutItemInstance *inst, QDomElement e);
    void clear();

    QList<LayoutScreen> screens;
    QList<LayoutSchedule> schedules;
};


#endif
