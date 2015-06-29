#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "wifi.h"
#include <string.h>
#include "protos.h"

extern struct ap_info accesspoints[];
LOCAL struct station_config sta_conf;

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

LOCAL void ICACHE_FLASH_ATTR setupWifiSta(const char *ssid, const char *pwd)
{
    memset(&sta_conf,0,sizeof(sta_conf));
    memcpy(&sta_conf.ssid, ssid, strlen(ssid));
    memcpy(&sta_conf.password, pwd, strlen(pwd));

    wifi_softap_dhcps_stop();

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_config(&sta_conf);
    wifi_station_disconnect();
    wifi_station_connect();
}


LOCAL void ICACHE_FLASH_ATTR wifi_ap_found_callback(struct ap_info *ap)
{
    os_printf("Connecting to AP: %s\n", ap->ssid);
    setupWifiSta( ap->ssid, ap->pwd );
}

LOCAL void ICACHE_FLASH_ATTR scan_done_cb(void *arg, STATUS status)
{
    scaninfo *c = arg;
    struct bss_info *inf;
//    os_printf("Scan done\n");

    if (status == OK) {
        if (!c->pbss) {
            return;
        }

        STAILQ_FOREACH(inf, c->pbss, next) {
  //          os_printf("Scan AP: %s\n", inf->ssid);
            struct ap_info *ap = wifi_get_ap((const char*)inf->ssid);
            if (ap) {
                wifi_ap_found_callback(ap);
                break;
            }
            inf = (struct bss_info *) &inf->next;
        }
    }
    // Schedule a new scan ?
    //    os_timer_arm(&wifi_scan_ap, 1000, 0);
    //    wifi_scan_ap();
}

void ICACHE_FLASH_ATTR wifi_scan_ap()
{
    os_printf("Scanning for Access Points\n");
   /* if (!wifi_station_scan(NULL, &scan_done_cb))
    os_printf("Cannot scan???\n");
    */
}
