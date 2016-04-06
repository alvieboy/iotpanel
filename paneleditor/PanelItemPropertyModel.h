#ifndef __PANELITEMPROPERTYMODEL_H__
#define __PANELITEMPROPERTYMODEL_H__

#include <QAbstractTableModel>
#include <QDebug>

class PanelProperty;

class PanelItemPropertyModel : public QAbstractTableModel
{
    //Q_OBJECT
public:
    PanelItemPropertyModel(QObject *parent) : QAbstractTableModel(parent), m_list(NULL) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        if (m_list==NULL)
            return 0;
        return m_list->size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        return 2;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return QString(tr("Name"));
            case 1:
                return QString(tr("Value"));
            default:
                break;
            }
        }
        return QVariant();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
#if 1
        if (m_list!=NULL) {
            qDebug()<<"setting props";
            if (role == Qt::DisplayRole && index.row()<m_list->size())
            {
                PanelProperty *i = (*m_list)[index.row()];
                if (index.column()==0) {
                    return i->name;
                }
                return "";
            }

        }
#endif
        return QVariant();
    }

    void setList(QList<PanelProperty*> *l) {
        beginResetModel();
        m_list=l;
        endResetModel();
        
    }
private:
    QList<PanelProperty*> *m_list;
};

#endif
