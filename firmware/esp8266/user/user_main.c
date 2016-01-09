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
#include "schedule.h"
#include "alloc.h"
#include "upgrade.h"
#include "wifi.h"
#include "clock.h"
#include "flash_serializer.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

#ifndef UART0
#define UART0 0
#endif

#ifndef HOST
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
os_event_t    broadcast_procTaskQueue[user_procTaskQueueLen];

static void user_procTask(os_event_t *events);
static volatile os_timer_t some_timer;
#endif

extern void draw_current_screen();
extern void spi_setup();
extern void timer_setup();

void user_rf_pre_init(void)
{
}

/* Framebuffer */
uint8_t framebuffer[32*32*HORIZONTAL_PANELS];
extern volatile int fbdone;

struct gfxinfo gfx =
{
    32*HORIZONTAL_PANELS, //stride
    32*HORIZONTAL_PANELS, //width
    32, //height
    &framebuffer[0]//fb
};

char *os_strdup(const char *c)
{
    char *d = os_malloc(strlen(c)+1);
    strcpy(d,c);
    return d;
}


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
    widget_t *w = widget_find("status");
    if (w!=NULL) {
        widget_set_property(w,"text", status);
    }
//    updateScrollingText( &scr, status);
}

static struct ip_info ip;

struct espconn conn_udpb;

LOCAL void ICACHE_FLASH_ATTR broadcastIP()
{
    uint32_t lip = ip.ip.addr;
    if (lip!=0) {
        unsigned char payload[10];
        unsigned int size = 0;
        payload[size++] = lip>>24;
        payload[size++] = lip>>16;
        payload[size++] = lip>>8;
        payload[size++] = lip;
        wifi_get_macaddr(STATION_IF, &payload[size]);
        size += 6;
        int i = espconn_sent( &conn_udpb, payload, size);
        os_printf("Sending new broadcast: %d\n",i);
    }
}

//static int broadcastRunning=0;

LOCAL void ICACHE_FLASH_ATTR newWifiStatus(int status, int oldstatus)
{
    char buf[64];

    os_printf("New WiFI status: %d (%d)\n", status, oldstatus);

    switch (status) {
    case STATION_IDLE:
        ip.ip.addr = 0;
        wifi_set_opmode(STATION_MODE);
        wifi_scan_ap();
        break;
    case STATION_CONNECTING:
        ip.ip.addr = 0;
        wifiUpdate("Connecting to WiFI Access Point");
        break;
    case STATION_WRONG_PASSWORD:
        wifiUpdate("Error connecting: bad password");
        ip.ip.addr = 0;
        wifi_set_opmode(STATION_MODE);
        wifi_scan_ap();
        break;
    case STATION_NO_AP_FOUND:
        wifiUpdate("Error connecting: no AP found");
        ip.ip.addr = 0;
        wifi_set_opmode(STATION_MODE);
        wifi_scan_ap();
        break;
    case STATION_CONNECT_FAIL:
        wifiUpdate("Connection failed, retrying");
        ip.ip.addr = 0;
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
        wifiUpdate(buf);
        {
            char buf2[64];
            system_rtc_mem_read( 0,buf2,64);
            int i;
            for (i=0;i<64;i++) {
                os_printf("%02x ",buf2[i]);
            }
            os_printf("\n");
        }


        break;
    default:
        break;
    }
}
#endif

void ICACHE_FLASH_ATTR redraw()
{
    schedule_event();
    draw_current_screen(&gfx);
}

#ifndef HOST

LOCAL unsigned tickcount = 0;

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    while (!fbdone) {
        system_os_post(user_procTaskPrio, 0, 0 );
    }
    fbdone=0;
            /*
    while (!fbdone) {
        system_os_post(user_procTaskPrio, 0, 0 );
    }
    fbdone=0;
              */
#if 1
    wifiConnect();

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
    time_tick();
    tickcount++;
    if ((tickcount&0x3ff)==0) {
        broadcastIP();
    }


    system_os_post(user_procTaskPrio, 0, 0 );
}
#endif
#if 0
static void setupFramebuffer()
{
    int x,y,p=0;
    for (x=0;x<32*HORIZONTAL_PANELS;x++) {
        for (y=0;y<32;y++) {
            framebuffer[p] = y & 0x7;
            p++;
        }
    }
}
#endif
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

#define ESC "\x1b"

extern char currentFw[];

LOCAL void ICACHE_FLASH_ATTR setupDefaultScreen()
{
    screen_destroy_all();

    if (deserialize_all(&flash_serializer)==0)
        return;
    os_printf("Using default screen\n");

    screen_destroy_all();
    
    screen_t *screen = screen_create("default");

    widget_t *sc = widget_create("scrollingtext","status");
    widget_set_property(sc, "font", "thumb" );
    widget_set_property(sc, "text", "IoT "
#if 1
                        ESC "c01"
                        "R"
                        ESC "c02"
                        "G"
                        ESC "c04"
                        "B"
                        ESC "cff"
#endif
                        " "
                        "Panel - (C) 2015 Alvie");
    widget_set_property(sc, "color", "white");
    widget_set_property(sc, "speed", "2");

    screen_add_widget(screen, sc, 0, 0);
#if 0
    widget_t *text = widget_create("text","example");
    widget_set_property(text, "font", "thumb" );
    widget_set_property(text, "wrap", "1" );
    widget_set_property(text, "width", "64" );
    widget_set_property(text, "text", "Para mais info contactar" );
    widget_set_property(text, "color", "yellow" );

    screen_add_widget(screen, text, 0, 8);
    text = widget_create("text","ger");
    widget_set_property(text, "font", "thumb" );
    widget_set_property(text, "wrap", "1" );
    widget_set_property(text, "width", "64" );
    widget_set_property(text, "text", "A gerencia" );
    widget_set_property(text, "color", "green" );

    screen_add_widget(screen, text, 0, 16+8);

    sc = widget_create("scrollingtext","info");
    widget_set_property(sc, "font", "thumb" );
    widget_set_property(sc, "text", "Para mais informacoes contacte "
                        "a gerencia deste estabelecimento");
    widget_set_property(sc, "color", "yellow");
    widget_set_property(sc, "speed", "3");

    screen_add_widget(screen, sc, 0, 1+32-7);


    widget_t *tx = widget_create("text","nome");
    widget_set_property(tx, "font", "6x10" );
    widget_set_property(tx, "color", "red");
    widget_set_property(tx, "text", "Snooker");

    screen_add_widget(screen, tx, 2, 7);

    tx = widget_create("text","bar");
    widget_set_property(tx, "font", "6x10" );
    widget_set_property(tx, "color", "white");
    widget_set_property(tx, "text", "Score");

    screen_add_widget(screen, tx, 33, 16);
#endif

    screen_select(screen);
}

#ifndef HOST
LOCAL esp_udp eudp;

LOCAL void ICACHE_FLASH_ATTR broadcast_setup()
{
    memset(&conn_udpb, 0, sizeof(conn_udpb));
    conn_udpb.type = ESPCONN_UDP;
    conn_udpb.state = ESPCONN_NONE;
    conn_udpb.proto.udp = &eudp;
    eudp.local_port = 8082;
    eudp.remote_port = 8082;
    eudp.local_ip[0] = 0;
    eudp.local_ip[1] = 0;
    eudp.local_ip[2] = 0;
    eudp.local_ip[3] = 0;
                               //10.8.10.44
    eudp.remote_ip[0] = 255;//250;
    eudp.remote_ip[1] = 255;//255;
    eudp.remote_ip[2] = 255;//255;
    eudp.remote_ip[3] = 255;//239;

    espconn_create(&conn_udpb);
}
#endif

LOCAL void upgrade_procTask(os_event_t *events)
{
    gpio_init();
    uart_setup();
    os_printf("Applying OTA upgrade.\n");
    apply_firmware();
    while (1) {
        system_os_post(0,0,0);
    }
}


void ICACHE_FLASH_ATTR user_init()
{
    if (has_new_firmware()) {
#ifndef HOST
        system_os_task(upgrade_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
        system_os_post(user_procTaskPrio, 0, 0 );
#endif
    } else {
#ifndef HOST
        gpio_init();
        gpio16_output_conf();
        gpio16_output_set(0); // GPIO16 low.
        spi_setup();
        timer_setup();
        uart_setup();
        os_printf("Last FW status: 0x%08x\n", get_last_firmware_status());
        broadcast_setup();

#endif
        //setupFramebuffer();
        //setupDefaultScreen();
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
        //setupFramebuffer();
        clearFramebuffer(&gfx);
        setupDefaultScreen();

        {
            char buf[64];
            system_rtc_mem_read( 0,buf,64);
            int i;
            for (i=0;i<64;i++) {
                os_printf("%02x ",buf[i]);
            }
            os_printf("\n");
        }



#ifdef HOST
        user_procTask(NULL);
#else
        system_os_task(user_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);

        system_os_post(user_procTaskPrio, 0, 0 );
#endif
    }
}
