#ifndef __WS_H__
#define __WS_H__

#define ENABLE_WEBSOCKET

#ifdef ENABLE_WEBSOCKET

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "smallfs.h"
#include "server.h"

enum httpstate {
    COMMAND,
    HEADER,
    DATA,
    REPLY,
    WEBSOCKET
};
enum websocketstate {
    WSCMD1,
    WSCMD2,
    WSLEN,
    WSMASK,
    WSDATA
};

#define WS_HDR1_FINBIT 0x80
#define WS_HDR2_MASKBIT 0x80

typedef struct websocket
{
    struct websocket *next;
    unsigned char *qdata;
    int qlen;
    struct espconn *conn;
    enum httpstate state;
    enum websocketstate wsstate;
    uint8_t close;
    char filename[16];
    char *reply;
    struct smallfsfile filetx;
    unsigned char wskey[32];
    uint64_t size;
    clientInfo_t *client;
    // Websocket proper
    uint8_t hdr1;
    uint8_t hdr2;
    uint8_t sizelen;
    uint8_t maskidx;
    uint8_t mask[4];
    uint8 upgrade:1; // Upgrade
    uint8 upgradews:1; // Upgrade to WS
} websocket_t;

int ws_data( struct espconn*, const unsigned char *, size_t);
void ws_connect( struct espconn* );
void ws_disconnect( struct espconn* );
void ws_datasent( struct espconn* );

#endif

#endif

