#include "mainwindow.h"
#include "Panel.h"
#include <QTimer>
#include <cstdio>

extern  "C"  {
#include "os_type.h"
#include "gfx.h"

    extern pixel_t *currentBuffer;
    extern uint8_t currentBufferId;
    void switchToNextBuffer();
}

#define LEDSIZE 6
#define LEDBORDER 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    resize(64*LEDSIZE, 32*LEDSIZE);
    m_panel = new Panel(this);
    setCentralWidget(m_panel);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateImage()));
    timer->start(20);
}

MainWindow::~MainWindow()
{
}


void MainWindow::updateImage()
{
    int x,y;


    if (currentBuffer) {
        PanelFramebuffer &fb = static_cast<Panel*>(m_panel)->fb();
        for (y=0;y<32;y++) {
            for (x=0;x<32*HORIZONTAL_PANELS;x++) {
                pixel_t color = currentBuffer[x+(y*32*HORIZONTAL_PANELS)];
                unsigned cr,cg,cb;
                cr = (color&0x07);
                cg = (color&0x31) >> 3;
                cb = (color&0xc0) >> 6;

                cr *= 255;
                cg *= 255;
                cb *= 255;
                cr/=7;
                cg/=7;
                cb/=3;
                if (color==0) {
                    cr=cg=cb=20;
                }
                fb.set(x,y,QColor(cr,cg,cb));
            }
        }
        bufferStatus[currentBufferId]=BUFFER_FREE;
        m_panel->update();
    }
    switchToNextBuffer();
}


