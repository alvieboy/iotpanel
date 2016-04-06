#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "wifi.h"
#include <string.h>
#include "protos.h"

extern struct ap_info accesspoints[];
LOCAL struct station_config sta_conf;
LOCAL volatile int wifi_do_reconnect = 0;

struct ap_info *ICACHE_FLASH_ATTR wifi_get_ap(const char *ssid)
{
    struct ap_info *ap = &accesspoints[0];
    while (ap->ssid) {
        if (strcmp(ssid,ap->ssid)==0)
            return ap;
        ap++;
    }
    return NULL;
}

LOCAL int ICACHE_FLASH_ATTR isAPMode()
{
    return 0;
}

void ICACHE_FLASH_ATTR wifiConnect()
{
    if (!wifi_do_reconnect)
        return;
    wifi_do_reconnect=0;
    if (isAPMode()) {
        struct softap_config config;
        wifi_station_disconnect();
        wifi_set_opmode(0x03);

        wifi_softap_get_config(&config); // Get config first.
        os_memset(config.ssid, 0, 32);
        os_memset(config.password, 0, 64);

        os_memcpy(config.ssid, "Panel", 5);
        os_memcpy(config.password, "ApP4ss4u", 8);

        config.authmode = AUTH_WPA_WPA2_PSK;
        config.ssid_len = 5;// or its actual length
        config.max_connection = 2; // how many stations can connect to ESP8266 softAP at most.

        wifi_softap_set_config(&config);// Set ESP8266 softap config
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
    sint8 bestrssi;
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
