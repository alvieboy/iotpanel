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
void ICACHE_FLASH_ATTR wifiConnect()
{
    if (!wifi_do_reconnect)
        return;
    wifi_do_reconnect=0;

    wifi_station_disconnect();
    wifi_softap_dhcps_stop();
    wifi_set_opmode(STATION_MODE);
    wifi_station_set_config(&sta_conf);
    wifi_station_connect();

}
void ICACHE_FLASH_ATTR setupWifiSta(const char *ssid, const char *pwd)
{
    memset(&sta_conf,0,sizeof(sta_conf));
    memcpy(&sta_conf.ssid, ssid, strlen(ssid));
    memcpy(&sta_conf.password, pwd, strlen(pwd));
    wifi_do_reconnect=1;
}


LOCAL void ICACHE_FLASH_ATTR wifi_ap_found_callback(struct ap_info *ap)
{
    os_printf("Connecting to AP: %s\n", ap->ssid);
    setupWifiSta( ap->ssid, ap->pwd );
}

void ICACHE_FLASH_ATTR scan_done_cb(void *arg, STATUS status)
{
    struct bss_info *bss_link = arg;
    int rescan = 1;

    if (status == OK) {

        struct bss_info *inf = bss_link->next.stqe_next;

        while (inf) {
            struct ap_info *ap = wifi_get_ap((const char*)inf->ssid);
            if (ap) {
                rescan=0;
                wifi_ap_found_callback(ap);
                break;
            }
            inf = inf->next.stqe_next;
        }
    } else {
        os_printf("Error scanning\n");
    }

    // Schedule a new scan, if we found nothing.
    if (rescan)
        wifi_scan_ap();
}

void ICACHE_FLASH_ATTR wifi_scan_ap()
{
    os_printf("Scanning for Access Points\n");
#if 1
    if (!wifi_station_scan(NULL, &scan_done_cb))
        os_printf("Cannot scan???\n");
#endif

}
