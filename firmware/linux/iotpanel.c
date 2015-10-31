#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "espconn.h"
#include <SDL2/SDL.h>
#include "gfx.h"
#include "serdes.h"
#include <sys/time.h>

#define LEDSIZE 6
#define LEDBORDER 1

espconn *current_conn = NULL;
static int listenfd = -1;
static struct timeval start;

#ifndef linux
typedef int socklen_t;
#endif

void ser_initialize(struct serializer_t *me)
{
}

int ser_write(struct serializer_t *me, const void *data, unsigned size)
{
    const unsigned char *cptr=data;
    printf("Write %d: ",size);
    while (size--) {
        printf("%02x ", *cptr);
        cptr++;
    }
    printf("\n");
    return 0;
}

int ser_read(struct serializer_t *me, void *data, unsigned size)
{
    return -1;
}
void ser_rewind(struct serializer_t *me)
{
}


struct serializer_t debug_serializer = {
    .write = &ser_write,
    .read = &ser_read,
    .rewind = &ser_rewind,
    .initialize = &ser_initialize
};

// Compats...



void *pvPortMalloc(size_t size)
{
    return malloc(size);
}
void *pvPortCalloc(size_t size, int n)
{
    return calloc(size,n);
}

void vPortFree(void *ptr)
{
    free(ptr);
}

void system_os_post()
{
    usleep(5000);
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

void (*cb_data)(void *, char *, unsigned short length) = NULL;
void (*cb_connect)(void*) = NULL;
void (*cb_disconn)(void*) = NULL;

int espconn_regist_recvcb(espconn*conn, void (*cb)(void *, char *, unsigned short length))
{
    cb_data = cb;
}

void espconn_regist_reconcb(espconn*conn, void (*cb)(void *arg, sint8 err))
{

}

void espconn_regist_connectcb(espconn*conn, void (*cb)(void*))
{
    cb_connect=cb;
}

void espconn_regist_disconcb(espconn*conn, void (*cb)(void*))
{
    cb_disconn=cb;
}

SDL_Window *win;
SDL_Renderer *ren;

int main(int argc,char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        return 1;
    }
    win = SDL_CreateWindow("IoT Panel", 100, 100, 32*HORIZONTAL_PANELS*LEDSIZE, 32*LEDSIZE, SDL_WINDOW_SHOWN);
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
    gettimeofday(&start,NULL);
    user_init();
}


extern const struct gfxinfo gfx;

void updateImage()
{
    SDL_Rect r;
    int x,y;

    SDL_SetRenderDrawColor(ren,0x0,0,0,0xff);
    SDL_RenderClear(ren);

    r.w=LEDSIZE-(LEDBORDER*2);
    r.h=LEDSIZE-(LEDBORDER*2);

    for (y=0;y<32;y++) {
        for (x=0;x<32*HORIZONTAL_PANELS;x++) {
            r.x=LEDBORDER + (x*LEDSIZE);
            r.y=LEDBORDER + (y*LEDSIZE);
            uint8_t color = gfx.fb[x+(y*32*HORIZONTAL_PANELS)];
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

static void clientData()
{
    unsigned char buf[8192];
    int r = read( current_conn->sockfd, buf, sizeof(buf));
    if (r<=0) {
        close(current_conn->sockfd);
        current_conn->sockfd=-1;
        cb_disconn(current_conn);
    } else {
        cb_data(current_conn,buf, r);
    }
}


static void newConnection()
{
    socklen_t len = sizeof(struct sockaddr_in);
    current_conn->sockfd = accept( listenfd, (struct sockaddr*)&current_conn->sock, &len);
    cb_connect(current_conn);
}


static void netCheck()
{
    struct timeval tv;
    fd_set rfs;
    int retry = 0;
    int max=-1;
    tv.tv_sec=0;
    tv.tv_usec=5000;

    do {
        FD_ZERO(&rfs);

        if (listenfd>=0) {
            FD_SET(listenfd,&rfs);
            if (max<listenfd)
                max=listenfd;
        }

        if (current_conn && current_conn->sockfd>=0) {
            FD_SET(current_conn->sockfd,&rfs);
            if (max<current_conn->sockfd)
                max=current_conn->sockfd;
        }
        switch(select(max+1, &rfs, NULL, NULL, &tv)) {
        case 0:
            retry=0;
            break;
        case -1:
            return;
        default:
            if (listenfd>=0 && FD_ISSET(listenfd,&rfs)) {
                newConnection();
            }
            if (current_conn && current_conn->sockfd>=0 &&
                FD_ISSET(current_conn->sockfd,&rfs)) {
                clientData();
            }
            break;
        }

    } while (retry);
}

void user_procTask(void*arg)
{
    SDL_Event e;
    int quit=0;
    while (!quit) {
        redraw();
        time_tick();
        updateImage();

        while (SDL_PollEvent(&e)){
            //If user closes the window
            if (e.type == SDL_QUIT){
                quit = 1;
            }
        }
#if 0
        // Test
        if (serc==1) {
            printf("Ser\n"),
            screen_serialize_all(&debug_serializer);
        }
        if (serc>0)
            serc--;
#endif
        netCheck();//os_delay_us(20000);
    }
}

void espconn_sent(espconn*conn, unsigned char *ptr, uint16_t size)
{
    send(conn->sockfd, ptr, size, 0);
}


uint32 system_get_time()
{
    struct timeval now, delta;
    gettimeofday(&now,NULL);
    timersub(&now, &start, &delta);
    return ((delta.tv_sec*1000000) + (delta.tv_usec));
}

void espconn_accept(espconn*conn)
{
    int yes=1;
    conn->sockfd=-1;
    current_conn = conn;
    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (listenfd<0) {
        perror("socket");
        abort();
    }
    memset(&conn->sock,0, sizeof(conn->sock));
    conn->sock.sin_family = AF_INET;
    conn->sock.sin_addr.s_addr = INADDR_ANY;
    conn->sock.sin_port = htons(conn->proto.tcp->local_port);
    if (bind(listenfd, (struct sockaddr*)&conn->sock,sizeof(conn->sock))<0) {
        perror("bind");
        abort();
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))<0) {
        perror("setsockopt");
        abort();
    }
    listen(listenfd,1);
}
