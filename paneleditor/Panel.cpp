#include "Panel.h"
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QMouseEvent>
#include "font.h"
#include "text.h"

#include "PanelWidgetText.h"
#include "PanelItemPropertyModel.h"

#define PIXELWIDTH 7
#define PIXELHEIGHT 7
#define PIXELBORDERH 1
#define PIXELBORDERV 1

#ifndef PANELDISPLAYNOEDIT

QList<PanelItem *> Panel::CheckIntercept( int x, int y )
{
    QList<PanelItem*> allItems;

    foreach (PanelItem *i, m_items) {
        QRect outline = i->getScreenRect( m_uWidth, m_uHeight);
        QPoint point(x,y);
        if (outline.contains(point))
            allItems.append(i);
    }
    return allItems;
}

int Panel::getItemIndex(const PanelItem *i)
{
    int idx = 0;
    foreach (PanelItem *p, m_items) {
        if (i==p)
            return idx;
        idx++;
    }
    return -1;
}

void Panel::mousePressEvent(QMouseEvent *ev)
{
    QList<PanelItem *> intercept;

   // qDebug()<<"Mouse press";
    if (grabbedItem) {
        grabbedItem->release();
        grabbedItem=NULL;
        update();
    } else {

        foreach (PanelItem *i, m_items) {
            i->release();
            i->deselect();
        }

        PanelItem * i = NULL;
        QList<PanelItem *> list = CheckIntercept(ev->x(), ev->y());
        if (list.size()==1) {
            i = list[0];
        }

        if (i) {
            /* Update list */
            emit itemGrabbed( getItemIndex(i) );
            if (grabbedItem) {
                grabbedItem->release();
                grabbedItem->deselect();
                grabbedItem=NULL;
            }
            if (ev->button() == Qt::RightButton) {
                grabx = ev->x();
                graby = ev->y();
                // Adjust with current posistion
                grabx -= i->getX()*PIXELWIDTH;
                graby -= i->getY()*PIXELHEIGHT;
                //qDebug()<<"Grabbed at "<<grabx<<graby;
                grabbedItem = i;
                i->grab();
            } else {
                i->select();
            }
        }
        update();
    }
}

void Panel::mouseReleaseEvent(QMouseEvent *)
{
    //qDebug()<<"Mouse release";
}


void Panel::mouseMoveEvent(QMouseEvent *ev)
{
    int x, y;
    if (grabbedItem) {
        //qDebug()<<"Mouse move"<< ev->x()<<ev->y();

        x=ev->x() - grabx;
        y=ev->y() - graby;
        // Convert to pix coords
        x/=PIXELWIDTH;
        y/=PIXELHEIGHT;
        if (x != grabbedItem->getX() || y != grabbedItem->getY()) {
            grabbedItem->move(x,y);
            update();
        }
    }
}

#endif

Panel::Panel(QWidget *parent): QWidget(parent), m_fb(64,32)
{
    m_uWidth = 64;
    m_uHeight = 32;
    setFixedSize( m_uWidth*PIXELWIDTH, m_uHeight*PIXELHEIGHT);
    background = new QBrush(QColor(0,0,0));
    grey = new QBrush(QColor(128,128,128));
    m_fb.clear(QColor(64,64,64));
    setFocusPolicy(Qt::StrongFocus); 
#ifndef PANELDISPLAYNOEDIT
    grabbedItem = NULL;
    setMouseTracking(true);

    PanelWidgetText *t  =new PanelWidgetText("results");
    t->setText("Annual results");
    PanelItem *i = new PanelItem(t, 4, 0);
    m_items.append(i);

    t  =new PanelWidgetText("location");
    t->setText("Portugal");
    i = new PanelItem(t, 12, 0);
    m_items.append(i);

    t  =new PanelWidgetText("value");
    t->setText("8.2 Million");
    i = new PanelItem(t, 12, 9);
    m_items.append(i);
#endif
}


#ifndef PANELDISPLAYNOEDIT
void Panel::paintItemLocation(QPainter &painter,  PanelItem *i)
{
    QBrush select = QBrush(QColor(128,128,0));

    QRect rect = i->getScreenRect(m_uWidth,m_uHeight);

    QBrush selectFill = QBrush( QColor(128,128,0));
    QBrush grabbedFill = QBrush( QColor(255,255,128));

    QPen pen(QColor(255,0,0));
    pen.setWidth(1);

    painter.setBrush(select);
    painter.setOpacity( i->isGrabbed() ? 0.9 : i->isSelected() ? 0.6: 0.3 );
    painter.drawRect(rect);

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.setOpacity( 1.0 );
    painter.drawRect(rect);
}
#endif

void Panel::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    QRect ledrect;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);


    painter.fillRect(event->rect(), *background);
#ifndef PANELDISPLAYNOEDIT
    m_fb.clear(QColor(64,64,64));

    painter.translate(0, 0);

    foreach (PanelItem *i, m_items) {
        paintItemLocation( painter, i);
    }

    foreach (PanelItem *i, m_items) {
        i->drawTo( m_fb);
    }
#endif
    unsigned x,y;
    for (x=0;x<m_uWidth;x++) {
        for (y=0;y<m_uHeight;y++) {
            ledrect = QRect( (x*PIXELWIDTH)+1, (y*PIXELHEIGHT)+1, PIXELWIDTH-2, PIXELHEIGHT-2 );
            painter.fillRect( ledrect, QBrush(m_fb.get(x,y)));
        }
    }
 
    //painter.save();
    painter.end();
}

QRect PanelItem::getScreenRect(unsigned maxwidth, unsigned maxheight) const
{
    int x = getX();
    int y = getY();
    int w = getClass()->getWidth();
    int h = getClass()->getHeight();

    if (w<0) {
        w = maxwidth - x;
    }
    if (h<0) {
        h = maxheight - h;
    }
    return QRect( x*PIXELWIDTH, y*PIXELHEIGHT, w*PIXELWIDTH, h*PIXELHEIGHT);
}

#ifndef PANELDISPLAYNOEDIT
extern PanelItemPropertyModel *pmodel;

void Panel::updateProperties(const PanelItem *item)
{
   // ui->propertyTable.clear();

    m_itemProperties = item->getClass()->getProperties();
    pmodel->setList(&m_itemProperties);

    update();

}

void Panel::onItemActivated(const QModelIndex &index)
{
    unsigned x = 0;
    qDebug()<<"Item activated "<<index;
    foreach (PanelItem *i, m_items) {
        if (index.row() ==x ) {
            i->select();
            updateProperties(i);
        } else {
            i->deselect();
        }
        x++;
    }
    update();
}

#endif

Panel::~Panel()
{
}
