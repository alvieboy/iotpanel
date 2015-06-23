#ifndef __ESPCONN_H__
#define __ESPCONN_H__

#ifdef __linux__

#include "os_type.h"
#include <netinet/in.h>

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
