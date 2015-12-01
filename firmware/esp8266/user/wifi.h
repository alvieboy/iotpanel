#ifndef __WIFI_H__
#define __WIFI_H__

struct ap_info {
    const char *ssid;
    const char *pwd;
};

#define END_OF_ACCESSPOINTS {0,0}

struct ap_info *wifi_get_ap(const char *name);
void wifi_scan_ap();
void wifiConnect();
void setupWifiSta(const char *,const char*);
#endif
