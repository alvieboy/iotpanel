#ifndef __PANELLAYOUT_H__
#define __PANELLAYOUT_H__

#include <QString>
#include <QPoint>
#include <QList>
#include <QFile>

class LayoutItemInstance
{
    QString name;
    QString description;
    QString classname;
    QMap<QString, QString> properties;
};

class LayoutItemEntry
{
    LayoutItemInstance *instance;
    QPoint pos;
};

class LayoutScreen
{
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
    int createFromFile(QFile &);
    int writeToFile(QFile &);

    QList<LayoutScreen> screens;
    QList<LayoutSchedule> schedules;
};


#endif
