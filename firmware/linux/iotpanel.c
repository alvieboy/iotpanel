#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "espconn.h"
#include <SDL2/SDL.h>
#include "gfx.h"

espconn *current_conn = NULL;

// Compats...



void *pvPortMalloc(size_t size)
{
    return calloc(size,1);
}

void vPortFree(void *ptr)
{
    free(ptr);
}

void system_os_post()
{
    usleep(20000);
}

void os_delay_us(int us)
{
    usleep(us);
}

int os_printf(const char *fmt,...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vprintf(fmt, ap);
    va_end(ap);
    return r;
}

int os_sprintf(char *dest, const char *fmt,...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsprintf(dest,  fmt, ap);
    va_end(ap);
    return r;
}


// Espconn stuff.

void espconn_regist_time()
{
}

int espconn_regist_recvcb(espconn*conn, void (*cb)(void *, char *, unsigned short length))
{
    return 0;
}

void espconn_regist_reconcb(espconn*conn, void (*cb)(void *arg, sint8 err))
{
    return 0;
}

void espconn_regist_connectcb(espconn*conn, void (*cb)(void*))
{
    return 0;
}

void espconn_regist_disconcb(espconn*conn, void (*cb)(void*))
{
    return 0;
}

SDL_Window *win;
SDL_Renderer *ren;

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        return 1;
    }
    win = SDL_CreateWindow("IoT Panel", 100, 100, 320, 320, SDL_WINDOW_SHOWN);
    if (win == NULL){
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == NULL){
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 1;
    }
    user_init();
}

#define LEDSIZE 10

extern const struct gfxinfo gfx;

void updateImage()
{
    SDL_Rect r;
    int x,y;

    SDL_SetRenderDrawColor(ren,0x0,0,0,0xff);
    SDL_RenderClear(ren);

    r.w=LEDSIZE-2;
    r.h=LEDSIZE-2;

    for (y=0;y<32;y++) {
        for (x=0;x<32;x++) {
            r.x=1 + (x*LEDSIZE);
            r.y=1 + (y*LEDSIZE);
            uint8_t color = gfx.fb[x+(y*32)];
            int cr,cg,cb;
            cr = color&1 ? 0xff:0x00;
            cg = color&2 ? 0xff:0x00;
            cb = color&4 ? 0xff:0x00;
            SDL_SetRenderDrawColor(ren,cr,cg,cb,0xff);
            SDL_RenderFillRect(ren,&r);
        }
    }
    
        //Draw the texture
    //SDL_RenderCopy(ren, tex, NULL, NULL);
    //Update the screen
    SDL_RenderPresent(ren);
}


void user_procTask(void*arg)
{
    SDL_Event e;
    int quit=0;
    while (!quit) {
        redraw();
        updateImage();

        while (SDL_PollEvent(&e)){
            //If user closes the window
            if (e.type == SDL_QUIT){
                quit = 1;
            }
        }
        os_delay_us(20000);
    }
}

void espconn_sent(espconn*conn, unsigned char *ptr, uint16_t size)
{
    send(conn->sockfd, ptr, size, 0);
}

void espconn_accept(espconn*conn)
{
}
