#ifndef __ESPCONN_H__
#define __ESPCONN_H__

#ifdef HOST

#include "os_type.h"
#ifdef __linux__
#include <netinet/in.h>
#else
#include <windows.h>
#endif

#define ESPCONN_TCP IPPROTO_TCP
#define ESPCONN_NONE IPPROTO_NONE

typedef struct esp_tcp {
    int local_port;
} esp_tcp;


typedef struct espconn {
    int type;
    int state;
    union {
        esp_tcp *tcp;
    } proto;
    struct sockaddr_in sock;
    int sockfd;
} espconn;


#endif

#endif
