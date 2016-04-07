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
#include "gfx.h"
#include "serdes.h"
#include <sys/time.h>
#include <fcntl.h>
#include "flash_serializer.h"
#include <unistd.h>
#include "framebuffer.h"
#include <string.h>

#define LEDSIZE 6
#define LEDBORDER 1

#ifdef USESDL
#include <SDL2/SDL.h>
#endif

espconn *current_conn = NULL;
static int listenfd = -1;
static struct timeval start;
static uint8_t currentDrawBuffer=0;
#ifndef linux
typedef int socklen_t;
#endif
int displayThread(void*);

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

void *pvPortRealloc(void *ptr, size_t size)
{
    return realloc(ptr,size);
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


static espconn_recv_callback cb_data;
static espconn_connect_callback cb_connect;
static espconn_connect_callback cb_disconn;
// Espconn stuff.

sint8 espconn_regist_recvcb(struct espconn*conn, espconn_recv_callback cb)
{
    cb_data = cb;
    return 0;
}

sint8 espconn_regist_reconcb(struct espconn*conn, void (*cb)(void *arg, sint8 err))
{
    return -1;
}

sint8 espconn_disconnect(struct espconn*conn)
{
    if (conn) {
        printf("Closing connection on fd %d\n", conn->sockfd);
        close(conn->sockfd);
        conn->sockfd=-1;
        return 0;
    } else {
        printf("Invalid CONNECTION!!!!\n");
    }
    return -1;
}

sint8 espconn_delete(struct espconn*conn)
{
#if 0
    if (conn) {
        printf("Closing connection on fd %d\n", conn->sockfd);
        close(conn->sockfd);
        conn->sockfd=-1;
    } else {
        printf("Invalid CONNECTION!!!!\n");
    }
#endif
    return 0;
}

sint8 espconn_regist_connectcb(struct espconn*conn, espconn_connect_callback cb)
{
    cb_connect=cb;
    return 0;
}

sint8 espconn_regist_disconcb(struct espconn*conn, void (*cb)(void*))
{
    cb_disconn=cb;
    return 0;
}

#ifdef USESDL
SDL_Window *win;
SDL_Renderer *ren;
SDL_Thread *thread;
#endif

extern void user_init();

#ifdef USESDL

extern struct gfxinfo gfx;
static uint8_t currentBufferId=1;
static pixel_t *currentBuffer=NULL;
static unsigned int ticks = 0;
#define TICKS_PER_BUFFER 2



static inline void switchToNextBuffer()
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

#endif
void setBlanking(int x)
{
}

int getBlanking()
{
    return 0xde;
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
        cb_data(current_conn,(char*)buf, r);
    }
}


static void newConnection()
{
    socklen_t len = sizeof(struct sockaddr_in);
    current_conn->sockfd = accept( listenfd, (struct sockaddr*)&current_conn->sock, &len);
    cb_connect(current_conn);
}


void netCheck()
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

extern void redraw();
extern void time_tick();

#ifdef USESDL
volatile int quit=0;

int displayThread(void*arg)
{
    while (!quit) {
        updateImage();
        usleep(10000);
    }
    return 0;
}

void user_procTask(void*arg)
{
    SDL_Event e;
    //static int serc = 10;
    while (!quit) {
        while (bufferStatus[currentDrawBuffer] != BUFFER_FREE) {
            //system_os_post(user_procTaskPrio, 0, 0 );
            usleep(10000);
        }

        gfx.fb = &framebuffers[currentDrawBuffer][0];
        //printf("Redrawing screen %d\n", currentDrawBuffer);
        redraw();

        bufferStatus[currentDrawBuffer] = BUFFER_READY;
        currentDrawBuffer ++;
        currentDrawBuffer&=1;

        time_tick();

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
            screen_serialize_all(&flash_serializer);
            deserialize(&flash_serializer);
        }
        if (serc>0)
            serc--;
#endif
        netCheck();//os_delay_us(20000);
    }
    int tstatus;
    SDL_WaitThread(thread,&tstatus);
}
#endif

sint8 espconn_sent(struct espconn*conn, unsigned char *ptr, uint16_t size)
{
    return send(conn->sockfd, ptr, size, 0);
}


uint32 system_get_time()
{
    struct timeval now, delta;
    gettimeofday(&now,NULL);
    timersub(&now, &start, &delta);
    return ((delta.tv_sec*1000000) + (delta.tv_usec));
}

sint8 espconn_regist_time(struct espconn *espconn, uint32 interval, uint8 type_flag)
{
    return 0;
}


sint8 espconn_accept(struct espconn*conn)
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
    return 0;
}

unsigned char rtcmem[ 128*4 ] = {0};

bool system_rtc_mem_read(uint8 src_addr, void *des_addr, uint16 load_size)
{
    int s = src_addr*4;
    memcpy(des_addr, &rtcmem[s], load_size);
    return true;
}
bool system_rtc_mem_write(uint8 des_addr, const void *src_addr, uint16 save_size)
{
    int s = des_addr*4;
    memcpy(&rtcmem[s], src_addr, save_size);
    return true;
}

// SPI flash
#define SECTORSIZE 4096

static int spiflashfd=-1;

static void openspi()
{
    if (spiflashfd<0) {
        spiflashfd = open("spi.bin",O_RDWR);
        if (spiflashfd<0) {
            perror("Cannot open spi.bin");
            abort();
        }
        ftruncate(spiflashfd, 128*SECTORSIZE);
    }
}

unsigned char _irom0_text_start;
unsigned char _irom0_text_end;

void system_restart()
{
    abort();
}

void uart_write_char(char c)
{
    if (c == '\n') {
        putchar('\r');
        putchar('\n');
    } else if (c == '\r') {
    } else {
        putchar(c);
    }

}

int spi_flash_erase_sector(uint16 sec)
{
    char sector[4096];
    openspi();
    uint32_t off = (uint32_t)sec * SECTORSIZE;
    lseek( spiflashfd, off, SEEK_SET);
    memset(sector, 0xff, sizeof(sector));
    write( spiflashfd, sector, sizeof(sector));
    return 0;
}
int spi_flash_write(uint32 des_addr, uint32 *src_addr, uint32 size)
{
    openspi();
    uint32_t off = des_addr;
    lseek( spiflashfd, off, SEEK_SET);
    write( spiflashfd, src_addr,size);
    return 0;
}
int spi_flash_read(uint32 src_addr, uint32 *des_addr, uint32 size)
{
    openspi();
    uint32_t off = src_addr;
    lseek( spiflashfd, off, SEEK_SET);
    read( spiflashfd, des_addr, size);
    return 0;
}

void CreateMutex(void *mutex)
{
}
bool GetMutex(void *mutex)
{
    return true;
}
void ReleaseMutex(void *mutex)
{
}
void LockMutex(void *mutex)
{
}

void panel_stop()
{
}

void *os_memcpy(void *dest, const void *src, size_t size)
{
    return memcpy(dest,src,size);
}
unsigned system_get_free_heap_size()
{
    return 16384;
}

void pp_soft_wdt_stop()
{
}

void ets_intr_lock()
{
}