#ifndef __PANELITEMMODEL_H__
#define __PANELITEMMODEL_H__

#include <QAbstractTableModel>

class PanelItemModel : public QAbstractTableModel
{
    //Q_OBJECT
public:
    PanelItemModel(QObject *parent) : QAbstractTableModel(parent), m_list(NULL) {}

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
                return QString(tr("Class"));
            case 1:
                return QString(tr("Name"));
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
            if (role == Qt::DisplayRole && index.row()<m_list->size())
            {
                PanelItem *i = (*m_list)[index.row()];
                if (index.column()==0) {
                    return i->getClass()->getClassName();
                }
                return i->getClass()->getName();
            }

        }
#endif
        return QVariant();
    }

    void setList(QList<PanelItem*> *l) {
        m_list=l;
    }
private:
    QList<PanelItem*> *m_list;
};

#endif
