#include "PanelLayout.h"
#include <QXmlStreamReader>
#include <QIODevice>
#include <QDomDocument>
#include <QDebug>

int PanelLayout::processItemProperties(LayoutItemInstance *inst, QDomElement e)
{
    // Parse properties
    QDomNodeList prop = e.childNodes();

    for (int i=0;i<prop.length();i++) {
        QDomNode n = prop.at(i);
        if (!n.isElement())
            continue;
        QString name = n.toElement().tagName();
        QString value = n.toElement().text();
        inst->properties[ name ] = value;
    }
    return 0;
}

int PanelLayout::processScheduleSelect(LayoutSchedule &schedule, QDomElement n)
{
    LayoutScheduleEntry e;
    e.command = LayoutScheduleEntry::SELECT;
    e.arg = QVariant(n.toElement().text());
    schedule.commands.append(e);
    return 0;
}

int PanelLayout::processScheduleWait(LayoutSchedule &schedule, QDomElement n)
{
    LayoutScheduleEntry e;
    e.command = LayoutScheduleEntry::WAIT;
    e.arg = QVariant(n.toElement().text().toInt());
    schedule.commands.append(e);
    return 0;
}

int PanelLayout::processScreenItem(LayoutScreen &screen, QDomElement e)
{
    LayoutItemEntry item;
    LayoutItemInstance *inst;

    inst = new LayoutItemInstance();

    inst->classname = e.attribute("class");
    inst->name = e.attribute("name");
    item.pos.setX(e.attribute("x").toInt());
    item.pos.setY(e.attribute("y").toInt());
    item.instance = inst;

    // Parse properties
    QDomNodeList prop = e.childNodes();

    for (int i=0;i<prop.length();i++) {
        QDomNode n = prop.at(i);
        if (!n.isElement())
            continue;
        QString name = n.toElement().tagName();
        if (name=="properties") {
            processItemProperties( inst, n.toElement() );
            continue;
        }
        if (name=="description") {
            inst->description = n.toElement().text();
            continue;
        }
        // Error
        return -1;
    }
    screen.items.append( item );
    return 0;
}

int PanelLayout::processSchedule(QDomElement node)
{
    LayoutSchedule schedule;

    schedule.name = node.attribute("name");
    if (schedule.name.length()==0)
        throw std::exception();

    QDomNodeList elements = node.childNodes();
    for (int i=0;i<elements.length();i++) {
        QDomNode n = elements.at(i);
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        if (e.tagName() == "description") {
            schedule.description = e.text();
            continue;
        }
        if (e.tagName() == "select") {
            processScheduleSelect( schedule, e );
            continue;
        }
        if (e.tagName() == "wait") {
            processScheduleWait( schedule, e );
            continue;
        }
        // Error
        return -1;
    }
    schedules.append(schedule);
    return 0;
}


int PanelLayout::processScreen(QDomElement node)
{
    LayoutScreen screen;
    // Get description
    screen.name = node.attribute("name");
    if (screen.name.length()==0)
        throw std::exception();

    QDomNodeList elements = node.childNodes();
    for (int i=0;i<elements.length();i++) {
        QDomNode n = elements.at(i);
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        if (e.tagName() == "description") {
            screen.description = e.text();
            continue;
        }
        if (e.tagName() == "item") {
            processScreenItem( screen, e );
            continue;
        }
        // Error
        return -1;
    }
    screens.append(screen);
    return 0;
}

int PanelLayout::process(QDomDocument &doc)
{
    qDebug()<<"Processing";

    QDomElement root = doc.documentElement();
    if (root.tagName() != "panelconfiguration") {
        throw std::exception();//("Ficheiro invalido");
    }

    QDomNodeList screens = root.elementsByTagName("screen");
    QDomNodeList schedules = root.elementsByTagName("schedule");

    for (int i = 0; i< screens.length(); i++) {
        processScreen( screens.at(i).toElement() );
    }
    for (int i = 0; i< schedules.length(); i++) {
        processSchedule( schedules.at(i).toElement() );
    }
    return 0;
}

int PanelLayout::createFromFile(QFile&f)
{
    QString error;
    QIODevice *dev = static_cast<QIODevice*>(&f);

    QDomDocument doc;
    if (!doc.setContent(dev))
        return -1;
    try {
        if (process(doc)!=0) {
            return -1;
        }
    } catch (std::exception e) {
        return -1;
    }
    return 0;
}


int PanelLayout::serialize(QStringList &dest)
{
    dest.clear();
    dest.append("WIPE 1");
    // Serialize screens
    foreach (const LayoutScreen &s, screens)
    {
        dest.append(QString("NEWSCREEN ")+ s.name);
        foreach (const LayoutItemEntry &item, s.items) {
            QStringList entry;
            entry.append("ADD");
            entry.append(s.name);
            entry.append(item.instance->classname);
            entry.append(item.instance->name);
            entry.append(QString::number(item.pos.x()));
            entry.append(QString::number(item.pos.y()));
            dest.append( entry.join(" ") );
            // Properties
            QMap<QString,QString>::const_iterator i;

            for (i = item.instance->properties.begin();
                 i != item.instance->properties.end();
                 i++) {
                QStringList pe;
                pe.append("PROPSET");
                pe.append(item.instance->name);
                pe.append(i.key());
                if (i.value().contains(" ")) {
                    pe.append(QString("\"")+ i.value() + "\"");
                } else {
                    pe.append(i.value());

                }
                dest.append( pe.join(" ") );
            }
        }
    }
    return 0;
}

int PanelLayout::serializeSchedule(const LayoutSchedule &s, QStringList &out)
{
    foreach (const LayoutScheduleEntry &e, s.commands) {
        switch (e.command) {
        case LayoutScheduleEntry::SELECT:
            out.append(QString("ADDSCHEDULE SELECT ")+e.arg.toString());
            break;
        case LayoutScheduleEntry::WAIT:
            out.append(QString("ADDSCHEDULE WAIT ")+QString::number(e.arg.toInt()));
            break;
        default:
            break;
        }
    }
    return 0;
}

void PanelLayout::clear()
{
    screens.clear();
    schedules.clear();
}

bool PanelLayout::isScreen(const QString &name)
{
    foreach (const LayoutScreen &s, screens) {
        if ( s.name == name)
            return true;
    }
    return false;
}
bool PanelLayout::isSchedule(const QString &name)
{
    foreach (const LayoutSchedule &s, schedules) {
        if ( s.name == name)
            return true;
    }
    return false;
}

const LayoutSchedule &PanelLayout::getSchedule(const QString &name)
{
    foreach (const LayoutSchedule &s, schedules) {
        if ( s.name == name)
            return s;
    }
    throw std::exception();
}
