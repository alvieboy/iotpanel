#ifndef __PANEL_H__
#define __PANEL_H__

#include <QWidget>
#include <QBrush>
#include <inttypes.h>
#include <QAbstractTableModel>

#include "PanelFramebuffer.h"
#include "PanelItem.h"
#include <QPainter>

class Panel : public QWidget {
    Q_OBJECT
  public:
      Panel(QWidget *parent = 0);
      virtual ~Panel();
  protected:
      void paintEvent(QPaintEvent *event);
#ifndef PANELDISPLAYNOEDIT
  protected:
      void paintItemLocation(QPainter&,  PanelItem*i);
      void mousePressEvent(QMouseEvent *);
      void mouseReleaseEvent(QMouseEvent *);
      void mouseMoveEvent(QMouseEvent *);
      QList<PanelItem*> CheckIntercept(int, int);
  public:
      QList<PanelItem*> *getItemList() { return &m_items; }
      QList<PanelProperty*> *getItemPropertyList() { return &m_itemProperties; }
      void onItemActivated(const QModelIndex &);
      int getItemIndex( const PanelItem *);
      void updateProperties( const PanelItem *);
#endif
  public:
      PanelFramebuffer &fb() { return m_fb; }
  protected:
      unsigned m_uWidth, m_uHeight; // in pixel
      QBrush *background, *grey;
      PanelFramebuffer m_fb;
      QList<PanelItem*> m_items;
      QList<PanelProperty*> m_itemProperties;

      // Grabbed stuff
      PanelItem *grabbedItem;
      int grabx, graby;
  signals:
      void itemGrabbed(int index);

};






#endif
