#include <QWindow>
#include <QApplication>
#include <QThread>
#include "mainwindow.h"
#include <unistd.h>
#include <cstdio>
#include <QDebug>
#include "Panel.h"
#include <QKeyEvent>

extern "C" {
#include "framebuffer.h"
#include "gfx.h"
}

static int quit = 0;

extern "C" {
    void user_init(void);
    void updateImage();
    void redraw();
    void time_tick();
    void netCheck();
    extern struct gfxinfo gfx;
    volatile uint8_t currentBufferId=1;
    pixel_t *currentBuffer=NULL;
    static unsigned int ticks = 0;
    static uint8_t currentDrawBuffer=0;
}


#define TICKS_PER_BUFFER 0



extern "C" void switchToNextBuffer()
{
    uint8_t nextBufferId = (currentBufferId+1)&1;
    if ((bufferStatus[nextBufferId] == BUFFER_READY) && ticks==TICKS_PER_BUFFER) {
        bufferStatus[currentBufferId] = BUFFER_FREE;
        currentBufferId = nextBufferId;
        bufferStatus[currentBufferId] = BUFFER_DISPLAYING;
        currentBuffer = &framebuffers[currentBufferId][0];
        ticks = 0;
    } else {
        if (ticks<TICKS_PER_BUFFER) {
            ticks++;
        }
    }
#if 0
    printf("Ticks: %d, currentBuffer %d\n", ticks, currentBufferId);
    printf("Buffer0 status: %d\n", bufferStatus[0]);
    printf("Buffer1 status: %d\n", bufferStatus[1]);
#endif
}


class RunnerThread: public QThread
{
    public:

        void run() {
            user_init();
        }
};


int main(int argc,char **argv)
{
    QApplication app(argc,argv);
    MainWindow w;
    w.show();

    RunnerThread thread;
    thread.start();
    app.exec();
    quit=1;
    thread.wait();
}

extern "C" void setup();
extern "C" void GAMELOOP();
extern "C" void GAMESTART();
int started=0;


int key_up=0;
int key_left=0;
int key_right=0;

extern "C" int isJump()
{
    return key_up;
}

extern "C" int isRight()
{
    return key_left;
}

extern "C" int isLeft()
{
    return key_right;
}

extern "C" unsigned tickcount;

extern "C" void user_procTask(void*arg)
{
    if(!started) {
        setup();
        GAMESTART();
        started=1;
    }

    while (!quit) {
     //   printf("Waiting for buffer %d\n", currentDrawBuffer);
        while (bufferStatus[currentDrawBuffer] != BUFFER_FREE) {
            //system_os_post(user_procTaskPrio, 0, 0 );
            usleep(10000);
            netCheck();
            if (quit)
                return;
        }
       // printf("Redrawing screen %d\n", currentDrawBuffer);

        gfx.fb = &framebuffers[currentDrawBuffer][0];
        redraw();
        if ((tickcount&0x3)==0)
            GAMELOOP();
        tickcount++;
        bufferStatus[currentDrawBuffer] = BUFFER_READY;
        currentDrawBuffer ++;
        currentDrawBuffer&=1;

        time_tick();

/*        while (SDL_PollEvent(&e)){
            //If user closes the window
            if (e.type == SDL_QUIT){
                quit = 1;
            }
        }*/
        netCheck();//os_delay_us(20000);
    }
    //int tstatus;
    //SDL_WaitThread(thread,&tstatus);
}
extern "C"  int zoom;

static void decreaseZoom()
{
    switch (zoom) {
    case 0:
        zoom=1;
        break;
    case 1:
        zoom=3;
        break;
    case 3:
        zoom=7;
        break;
    }
}

static void increaseZoom()
{
    switch (zoom) {
    case 7:
        zoom=3;
        break;
    case 3:
        zoom=1;
        break;
    case 1:
        zoom=0;
        break;
    }
}


void Panel::keyPressEvent( QKeyEvent *k )
{
    switch ( k->key()) {
    case Qt::Key_Left:
        key_left=1;
        break;
    case Qt::Key_Right:
        key_right=1;
        break;
    case Qt::Key_Up:
        key_up=1;
        break;
    case Qt::Key_Z:
        increaseZoom();
        break;
    case Qt::Key_X:
        decreaseZoom();
        break;
    default:
        break;
    }
}

void Panel::keyReleaseEvent( QKeyEvent *k )
{
    switch ( k->key()) {
    case Qt::Key_Left:
        key_left=0;
        break;
    case Qt::Key_Right:
        key_right=0;
        break;
    case Qt::Key_Up:
        key_up=0;
        break;
    default:
        break;
    }
}

