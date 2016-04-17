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

enum wsstate {
    COMMAND,
    HEADER,
    DATA,
    REPLY,
    WEBSOCKET
};

typedef struct websocket
{
    struct websocket *next;
    unsigned char *qdata;
    int qlen;
    struct espconn *conn;
    enum wsstate state;
    uint8_t close;
} websocket_t;

int ws_data( struct espconn*, const unsigned char *, size_t);
void ws_connect( struct espconn* );
void ws_disconnect( struct espconn* );
void ws_datasent( struct espconn* );

#endif

#endif

