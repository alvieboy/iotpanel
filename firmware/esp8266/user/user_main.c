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
#include <string.h>
#include "protos.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

#ifndef UART0
#define UART0 0
#endif

#ifndef HOST
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);
static volatile os_timer_t some_timer;
#endif

extern void draw_current_screen();
extern void spi_setup();
extern void timer_setup();

/* Framebuffer */
uint8_t framebuffer[32*32];
extern volatile int fbdone;

struct gfxinfo gfx =
{
    32, //stride
    32, //width
    32, //height
    &framebuffer[0]//fb
};


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

#ifndef HOST

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
        wifi_set_opmode(STATION_MODE);
        wifi_scan_ap();
        break;
    case STATION_CONNECTING:
        wifiUpdate("Connecting to WiFI Access Point");
        break;
    case STATION_WRONG_PASSWORD:
        wifiUpdate("Error connecting: bad password");
        break;
    case STATION_NO_AP_FOUND:
        wifiUpdate("Error connecting: no AP found");
        wifi_set_opmode(STATION_MODE);
        wifi_scan_ap();
        break;
    case STATION_CONNECT_FAIL:
        wifiUpdate("Connection failed, retrying");
        wifi_set_opmode(STATION_MODE);
        wifi_scan_ap();
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
#endif

void ICACHE_FLASH_ATTR redraw()
{
    draw_current_screen(&gfx);
}

#ifndef HOST

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
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
    if (laststatus<0) {
        laststatus=status;
        newWifiStatus(status, laststatus);
    }

    if (laststatus!=status) {
        newWifiStatus(status, laststatus);
    }
    laststatus=status;
#endif
    redraw();
    os_delay_us(5000);

    system_os_post(user_procTaskPrio, 0, 0 );
}
#endif

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

#ifndef HOST

#endif

LOCAL void ICACHE_FLASH_ATTR setupDHCPServer()
{
#ifndef HOST
    struct ip_info info;
    info.ip.addr = 0x0A0A0A0A;
    info.gw.addr = 0x0A0A0A0A;
    info.netmask.addr = 0x00FFFFFF;
    wifi_softap_dhcps_stop();
    wifi_set_ip_info(SOFTAP_IF, &info);
    wifi_softap_dhcps_start();
#endif
}


void ICACHE_FLASH_ATTR setupWifiAp(const char *ssid, const char *password)
{
#ifndef HOST
    struct softap_config config;

    int ssidlen = strlen(ssid);

    wifi_softap_get_config(&config);
    os_memcpy(config.ssid,ssid,ssidlen);
    config.ssid_len = ssidlen;
    config.ssid_hidden = 0;
    config.channel = 10;
    strcpy((char*)config.password, password);
    config.authmode = AUTH_WPA_WPA2_PSK;

    wifi_station_dhcpc_stop();

    wifi_softap_set_config(&config);
    /* Setup DHCP */
    setupDHCPServer();

    wifi_set_opmode(STATIONAP_MODE);
#endif
}

LOCAL void ICACHE_FLASH_ATTR
uart_setup()
{
#ifndef HOST
    uart_init_single(UART0, BIT_RATE_115200, 1);

    WRITE_PERI_REG(UART_INT_ENA(0), 0);
#endif
}

extern void user_server_init(uint32 port);

LOCAL void ICACHE_FLASH_ATTR setupDefaultScreen()
{
    int i;

    screen_t *screen = screen_create("default");

    widget_t *sc = widget_create("scrollingtext","sc");
    widget_set_property(sc, "font", "thumb" );
    widget_set_property(sc, "text", "IoT Panel demo - (C) 2015 Alvie");
    widget_set_property(sc, "color", "white");

    screen_add_widget(screen, sc, 0, 0);

    sc = widget_create("rectangle", "re1");
    widget_set_property(sc, "width", "30");
    widget_set_property(sc, "height", "17");
    widget_set_property(sc, "fill", "1");
    widget_set_property(sc, "color", "red");
    widget_set_property(sc, "bordercolor", "red");
    widget_set_property(sc, "altcolor", "yellow");
    widget_set_property(sc, "border", "1");
    widget_set_property(sc, "flash", "30");
    screen_add_widget(screen, sc, 1, 14);

    sc = widget_create("text", "txt");
    widget_set_property(sc, "font", "thumb" );
    widget_set_property(sc, "text", "10:10:10");
    widget_set_property(sc, "color", "yellow");

    screen_add_widget(screen, sc, 0, 7);
}


//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
#ifndef HOST
    gpio_init();
    gpio16_output_conf();
    gpio16_output_set(0); // GPIO16 low.
    spi_setup();
    timer_setup();
    uart_setup();
#endif
    setupFramebuffer();
    setupDefaultScreen();
#ifndef HOST
    setupWifiSta("","");
#endif
    user_server_init(8081);

#ifndef HOST
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15); // GPIO15.
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12);  // DI

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

    GPIO_OUTPUT_SET(4, 0);
#endif

    clearFramebuffer(&gfx);
//    setupWifiAp("IOTPANEL","alvie");
    setupDefaultScreen();

    //Start os task
#ifndef HOST
    os_delay_us(5000000);
#endif


#ifdef HOST
    user_procTask(NULL);
#else
    system_os_task(user_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
    system_os_post(user_procTaskPrio, 0, 0 );
#endif
}
