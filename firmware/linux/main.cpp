#include <QWindow>
#include <QApplication>
#include <QThread>
#include "mainwindow.h"
#include <unistd.h>
#include <cstdio>
#include <QDebug>

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
extern "C" void user_procTask(void*arg)
{
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
