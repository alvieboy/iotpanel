#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "wifi.h"
#include <string.h>
#include "protos.h"
#include "flash_serializer.h"
#include "error.h"
#include "alloc.h"

void wifi_scan_ap();

LOCAL struct ap_info *aplist = NULL;
LOCAL struct station_config sta_conf;
LOCAL volatile int wifi_do_reconnect = 0;

LOCAL uint8 ap_ssid[33];
LOCAL uint8 ap_pwd[65];
LOCAL uint8 apmode = 0;

#define FLASH_AP_START 0x7B

LOCAL void ICACHE_FLASH_ATTR wifi_ap_list_free()
{
    while (aplist) {
        struct ap_info *entry = aplist;
        aplist = entry->next;
        os_free(entry);
    }
}

LOCAL void ICACHE_FLASH_ATTR wifi_ap_list_append(struct ap_info *ap)
{
    ap->next = NULL;
    if (NULL==aplist) {
        aplist = ap;
    } else {
        struct ap_info *entry = aplist;
        while (entry->next) {
            entry = entry->next;
        }
        entry->next = ap;
    }
}

LOCAL unsigned ICACHE_FLASH_ATTR wifi_ap_list_count()
{
    unsigned count = 0;

    struct ap_info *entry = aplist;
    while (entry) {
        entry = entry->next;
        count++;
    }
    return count;
}

LOCAL int ICACHE_FLASH_ATTR wifi_load_ap_from_flash()
{
    serializer_t *ser = &flash_serializer;
    int r;
    unsigned sz;
    uint8_t numaps;

    wifi_ap_list_free();

    do {
        r = ser->initialize(ser, FLASH_AP_START);
        if (r!=NOERROR) break;
        // First, default AP ssid and password.
        r = deserialize_string(ser, (char*)ap_ssid, &sz, 32);
        if (r!=NOERROR) break;
        r = deserialize_string(ser, (char*)ap_pwd, &sz, 64);
        if (r!=NOERROR) break;
        // Number of AP in list
        r = deserialize_uint8(ser, &numaps);
        if (r!=NOERROR) break;
        while (numaps) {
            struct ap_info *newap = os_malloc(sizeof(struct ap_info));
            if (NULL==newap) {
                r=ENOMEM;
                break;
            }
            r = deserialize_string(ser, newap->ssid, &sz, 32);
            if (r!=NOERROR) break;
            r = deserialize_string(ser, newap->pwd, &sz, 64);
            if (r!=NOERROR) break;

            wifi_ap_list_append(newap);
            numaps--;
        }

    } while (0);

    if (r!=NOERROR) {
        os_printf("Error loading AP configuration, using defaults\n");
        // Free any AP list we might have.
        wifi_ap_list_free();
        // Fill in defaults.
        os_memcpy(ap_ssid, "Panel", 6);
        os_memcpy(ap_pwd,  "ApP4ss4ul", 10);
    }
    return r;
}

LOCAL int ICACHE_FLASH_ATTR wifi_save_ap_to_flash()
{
    serializer_t *ser = &flash_serializer;
    int r;
    uint8_t numaps;
    struct ap_info *ap = aplist;
    os_printf("Saving AP information to flash\n");
    do {
        r = ser->initialize(ser, FLASH_AP_START);
        if (r!=NOERROR) break;
        r = ser->truncate(ser);
        if (r!=NOERROR) break;
        // First, default AP ssid and password.
        r = serialize_string(ser, (char*)ap_ssid);
        if (r!=NOERROR) break;
        r = serialize_string(ser, (char*)ap_pwd);
        if (r!=NOERROR) break;
        // Number of AP in list
        numaps = wifi_ap_list_count();
        r = serialize_uint8(ser, numaps);
        if (r!=NOERROR) break;

        while (numaps) {
            r = serialize_string(ser, ap->ssid);
            if (r!=NOERROR) break;
            r = serialize_string(ser, ap->pwd);
            if (r!=NOERROR) break;
            ap = ap->next;
            numaps--;
        }
    } while (0);

    r = ser->finalise(ser);
    ser->release(ser);
    return r;
}

struct ap_info *ICACHE_FLASH_ATTR wifi_get_ap(const char *ssid)
{
    struct ap_info *ap = aplist;
    while (ap) {
        if (strcmp(ssid,ap->ssid)==0)
            return ap;
        ap = ap->next;
    }
    return NULL;
}

int ICACHE_FLASH_ATTR wifi_add_ap(const char *ssid, const char *pwd)
{
    if (wifi_get_ap(ssid)!=NULL)
        return EALREADY;

    struct ap_info *newap = os_malloc(sizeof(struct ap_info));
    int r = NOERROR;
    if (NULL==newap) {
        r = ENOMEM;
    } else {
        strncpy(newap->ssid,ssid,32);
        strncpy(newap->pwd, pwd, 64);
        wifi_ap_list_append(newap);
        r = wifi_save_ap_to_flash();
    }
    return r;
}

int ICACHE_FLASH_ATTR wifi_remove_ap(const char *ssid)
{
    if (wifi_get_ap(ssid)==NULL)
        return ENOTFOUND;
    return ENOTFOUND;
}

LOCAL int ICACHE_FLASH_ATTR isAPMode()
{
    return apmode;
}

extern void pp_soft_wdt_restart();

void ICACHE_FLASH_ATTR wifiConnect()
{
    int restart=0;

    if (!wifi_do_reconnect)
        return;
    wifi_do_reconnect=0;
    if (isAPMode()) {
        struct softap_config config;

        wifi_station_disconnect();

        os_printf("Current mode: %d\n", wifi_get_opmode());


        os_memset(&config, 0, sizeof(config));

        wifi_softap_get_config(&config); // Get config first.

        os_printf("Settings: ssid %s, channel %d\n", config.ssid, config.channel);
        if (config.ssid[0]=='E') {
            restart=1;
        }
        os_memset(config.ssid, 0, 32);
        os_memset(config.password, 0, 64);

        os_memcpy(config.ssid, "Panel", 5);
        os_memcpy(config.password, "ApP4ss4ul", 9);

        config.authmode = AUTH_WPA_PSK;
        config.ssid_len = 5;// or its actual length
        config.max_connection = 2; // how many stations can connect to ESP8266 softAP at most.
        //config.channel = 4;
        config.beacon_interval = 100;

        os_printf("New settings: ssid %s, channel %d\n", config.ssid, config.channel);

        ETS_UART_INTR_DISABLE();
        wifi_softap_set_config(&config);// Set ESP8266 softap config
        ETS_UART_INTR_ENABLE();

        if (restart) {
            pp_soft_wdt_restart();
            wifi_set_opmode(SOFTAP_MODE);
            system_restart();
        }

    } else {
        wifi_station_disconnect();
        wifi_softap_dhcps_stop();
        wifi_set_opmode(STATION_MODE);
        wifi_station_set_config(&sta_conf);
        wifi_station_connect();
    }

}
void ICACHE_FLASH_ATTR setupWifiSta(const char *ssid, const char *pwd, const uint8_t *bssid)
{
    memset(&sta_conf,0,sizeof(sta_conf));
    memcpy(&sta_conf.ssid, ssid, strlen(ssid));
    memcpy(&sta_conf.password, pwd, strlen(pwd));
    if (bssid) {
        sta_conf.bssid_set = 1;
        memcpy(&sta_conf.bssid, bssid, 6);
    } else {
        sta_conf.bssid_set = 0;
    }
    wifi_do_reconnect=1;
}


LOCAL void ICACHE_FLASH_ATTR wifi_ap_found_callback(struct ap_info *ap, const uint8_t *bssid)
{
    os_printf("Connecting to AP: %s\n", ap->ssid);
    setupWifiSta( ap->ssid, ap->pwd, bssid );
}

void ICACHE_FLASH_ATTR scan_done_cb(void *arg, STATUS status)
{
    struct bss_info *bss_link = arg;
    int rescan = 1;
    struct ap_info *best = NULL;
    sint8 bestrssi = -128;
    const uint8_t *bssid;

    if (status == OK) {

        struct bss_info *inf = bss_link->next.stqe_next;

        while (inf) {
            struct ap_info *ap = wifi_get_ap((const char*)inf->ssid);
            if (ap) {
                rescan=0;
                if (best == NULL) {
                    bssid = inf->bssid;
                    best = ap;
                    bestrssi = inf->rssi;
                } else {
                    if (inf->rssi > bestrssi) {
                        bssid = inf->bssid;
                        best = ap;
                        bestrssi = inf->rssi;
                    }
                }
            }
            inf = inf->next.stqe_next;
        }
    } else {
        os_printf("Error scanning\n");
    }

    if (best)
        wifi_ap_found_callback(best, bssid);

    // Schedule a new scan, if we found nothing.
    if (rescan)
        wifi_scan_ap();
}

void ICACHE_FLASH_ATTR wifi_init()
{
    int r = wifi_load_ap_from_flash();
    if (r!=NOERROR) {
        /* Start AP mode right away */
        apmode = 1;
//        wifiConnect();
    } else {

    }
    wifi_do_reconnect = 1;
}

void ICACHE_FLASH_ATTR wifi_idle()
{
    wifiConnect();
    if (!apmode)
        wifi_scan_ap();
}

void ICACHE_FLASH_ATTR wifi_scan_ap()
{
    if (isAPMode())
        return;
    os_printf("Scanning for Access Points\n");
#if 1
    if (!wifi_station_scan(NULL, &scan_done_cb))
        os_printf("Cannot scan???\n");
#endif

}
