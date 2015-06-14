#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "driver/gpio16.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "user_interface.h"
#include "espconn.h"
#include <stdio.h>
#include "gfx.h"
#include "wifi-config.h"
#include "widget.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

#ifndef UART0
#define UART0 0
#endif

os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

extern void draw_current_screen();

static volatile os_timer_t some_timer;

/* Framebuffer */
uint8_t framebuffer[32*32];
volatile int fbdone=0;

const struct gfxinfo gfx =
{
    32, //stride
    32, //width
    32, //height
    &framebuffer[0]//fb
};

static volatile int column=0;
static int ptr=0;
static int row=0;
static int holdoff=0;

#define HOLDOFF 6

#define CPLDCS 5 /* GPIO4?? */

static inline void myspi_master_9bit_write(uint8 spi_no, uint8 high_bit, uint8 low_8bit)
{
    uint32 regvalue;
    uint8 bytetemp;

    if (spi_no > 1) {
        return;
    }

    if (high_bit) {
        bytetemp = (low_8bit >> 1) | 0x80;
    } else {
        bytetemp = (low_8bit >> 1) & 0x7f;
    }

    regvalue = 0x80000000 | ((uint32)bytetemp);		//configure transmission variable,9bit transmission length and first 8 command bit

    if (low_8bit & 0x01) {
        regvalue |= BIT15;    //write the 9th bit
    }
    WRITE_PERI_REG(SPI_FLASH_USER2(spi_no), regvalue);				//write  command and command length into spi reg
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);		//transmission start
}


static inline void spi_select(int selected)
{
    GPIO_OUTPUT_SET(CPLDCS, selected);
}

static inline void spi_wait_transmission()
{
    while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);
}

//Do nothing function

static const char digits[]="0123456789";


static void clearFramebuffer()
{
    memset(framebuffer,0,sizeof(framebuffer));
}

void simpleitoa(int val, char *dest)
{
    char str[32];
    char *strptr=&str[30];
    str[31]=0;

    if (val<0) {
        *dest++='-';
        val=-val;
    }
    do {
        int rem = val%10;
        val/=10;
        *strptr = digits[rem];
        strptr--;
    } while (val);
    strptr++;
    strcpy(dest,strptr);
}

static volatile int count = 0;

int xoffset=0;
int laststatus = -1;

LOCAL void ICACHE_FLASH_ATTR wifiUpdate(const char *status)
{
//    updateScrollingText( &scr, status);
}

LOCAL void ICACHE_FLASH_ATTR newWifiStatus(int status, int oldstatus)
{
    struct ip_info ip;
    char buf[64];

    os_printf("New WiFI status: %d (%d)\n", status, oldstatus);

    switch (status) {
    case STATION_IDLE:
        break;
    case STATION_CONNECTING:
        wifiUpdate("Connecting to WiFI Access Point");
        break;
    case STATION_WRONG_PASSWORD:
        wifiUpdate("Error connecting: bad password");
        break;
    case STATION_NO_AP_FOUND:
        wifiUpdate("Error connecting: no AP found");
        break;
    case STATION_CONNECT_FAIL:
        wifiUpdate("Connection failed, retrying");
        break;
    case STATION_GOT_IP:
        wifi_get_ip_info(STATION_IF, &ip);
        os_sprintf(buf,"IP:%d.%d.%d.%d",
                   (ip.ip.addr>>0) & 0xff,
                   (ip.ip.addr>>8) & 0xff,
                   (ip.ip.addr>>16) & 0xff,
                   (ip.ip.addr>>24) & 0xff
                  );
        wifiUpdate(buf );
        break;
    default:
        break;
    }
}

LOCAL void ICACHE_FLASH_ATTR redraw()
{
    draw_current_screen(&gfx);
}

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    //char text[10];
    //int len;

    while (!fbdone) {
        system_os_post(user_procTaskPrio, 0, 0 );
    }
    fbdone=0;

    while (!fbdone) {
        system_os_post(user_procTaskPrio, 0, 0 );
    }
    fbdone=0;

#if 1
    int status = wifi_station_get_connect_status();
    if (laststatus<0)
        laststatus=status;

    if (laststatus!=status) {
        newWifiStatus(status, laststatus);
    }
    laststatus=status;
#endif
    redraw();
    os_delay_us(1000);

    system_os_post(user_procTaskPrio, 0, 0 );
}

static void 
spi_setup()
{
    spi_master_init(HSPI);
    uint32 regvalue = READ_PERI_REG(SPI_FLASH_CTRL2(HSPI));
    WRITE_PERI_REG(SPI_FLASH_CTRL2(HSPI),regvalue);
    WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI), 0x43043); //clear bit 31,set SPI clock div
}

#define FRC1_ENABLE_TIMER  BIT7

typedef enum {
    DIVDED_BY_1 = 0,
    DIVDED_BY_16 = 4,
    DIVDED_BY_256 = 8,
} TIMER_PREDIVED_MODE;

typedef enum {
    TM_LEVEL_INT = 1,
    TM_EDGE_INT   = 0,
} TIMER_INT_MODE;


static inline void strobe_set(int val)
{
    GPIO_OUTPUT_SET(12, val);
}

static inline void strobe()
{
    strobe_set(1);
    strobe_set(0);
}

LOCAL void tim1_intr_handler()
{
    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, 80);

    if (holdoff>0) {
        holdoff--;
        return;
    }

    if (column==0)
        spi_select(0);

    if (column<=31) {
        /* get pixel, upper row */
        uint32 regval = framebuffer[ptr];
        regval <<= 3;
        regval |= framebuffer[ptr+(16*32)] & 0x7;
        regval<<=1;
        regval|=0x80;
        myspi_master_9bit_write(HSPI, 1, regval);
        ptr++;
        column++;
    } else if (column==32) {
        // force clock high.
        myspi_master_9bit_write(HSPI, 1, 0x00);
        column++;
    } else if (column==33) {
        // Send row.
        myspi_master_9bit_write(HSPI, 0, row&0xf);
        // Strobe.
        column=0;
        // Disable OE
        GPIO_OUTPUT_SET(4, 1);

        strobe();
        // We need to wait here.
        while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);
        // Deselect.
        spi_select(1);
        // Enable OE again
        GPIO_OUTPUT_SET(4, 0);

        column=0;

        if ((row&0xf)==0xf) {
            ptr=0;
            holdoff = HOLDOFF * 32;
            fbdone=1;
        }
        row++;
    } 


}


static void timer_setup()
{
    ETS_FRC_TIMER1_INTR_ATTACH(tim1_intr_handler, NULL);
    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();

    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
    RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                  DIVDED_BY_16
                  | FRC1_ENABLE_TIMER
                  | TM_EDGE_INT);
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, 80);
}

static void setupFramebuffer()
{
    int x,y,p=0;
    for (x=0;x<32;x++) {
        for (y=0;y<32;y++) {
            framebuffer[p] = 0;//y & 0x7;
            p++;
        }
    }
}

LOCAL struct station_config sta_conf;

void setupWifi()
{
    memset(&sta_conf,0,sizeof(sta_conf));
    memcpy(&sta_conf.ssid, WIFI_SSID, strlen(WIFI_SSID));
    memcpy(&sta_conf.password, WIFI_PWD, strlen(WIFI_PWD));

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_config(&sta_conf);
    wifi_station_disconnect();
    wifi_station_connect();
}

LOCAL void ICACHE_FLASH_ATTR
uart_setup()
{
    uart_init_single(UART0, BIT_RATE_115200, 1);

    WRITE_PERI_REG(UART_INT_ENA(0), 0);

}

extern void user_server_init(uint32 port);

LOCAL void ICACHE_FLASH_ATTR setupDefaultScreen()
{
    int i;

    screen_t *screen = screen_create("default");
    widget_t *sc = widget_create("scrollingtext");
    widget_set_property(sc, "font", "thumb" );
    widget_set_property(sc, "text", "IoT Panel demo - (C) 2015 Alvie");
    widget_set_property(sc, "color", "white");

    screen_add_widget(screen, sc, 0, 0);

    //setupScrollingText(&scr, &gfx, font_find("thumb"), 0, "IoT Panel demo - (C) 2015 Alvie");
}


//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    // Initialize the GPIO subsystem.
    gpio_init();
//    gpio_output_set(0, BIT4|BIT12, BIT4|BIT12, 0);

    gpio16_output_conf();
    gpio16_output_set(0); // GPIO16 low.

    setupFramebuffer();
    setupDefaultScreen();

    spi_setup();
    timer_setup();
    uart_setup();
    user_server_init(8081);


    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15); // GPIO15.
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12);  // DI

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

    GPIO_OUTPUT_SET(4, 0);

    clearFramebuffer(&gfx);

    setupWifi();

    setupDefaultScreen();

    //Start os task
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    system_os_post(user_procTaskPrio, 0, 0 );

}
