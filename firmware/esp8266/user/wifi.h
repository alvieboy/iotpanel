#ifndef __WIFI_H__
#define __WIFI_H__

struct ap_info {
    struct ap_info *next;
    char ssid[33];
    char pwd[65];
};

#define END_OF_ACCESSPOINTS {0,0}

struct ap_info *wifi_get_ap(const char *name);
void wifiConnect();
void setupWifiSta(const char *,const char*, const uint8_t *bssid);
int wifi_add_ap(const char *ssid, const char *pwd);
void wifi_init();
void wifi_idle();

#endif
