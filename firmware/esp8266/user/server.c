#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include <stdio.h>
#include "widget.h"
#include <ctype.h>
#include <string.h>

LOCAL esp_tcp esptcp;
LOCAL struct espconn esp_conn;

#define MAX_LINE_LEN 256

typedef struct {
    char rline[MAX_LINE_LEN+1];
    char tline[MAX_LINE_LEN+1];
    unsigned  rlinepos;
    // Command placeholders.
    char *cmd;
    char *args;
    int argc;
    unsigned char authtoken[10];
    unsigned  authenticated:1;
    char *argv[8];

} clientInfo_t;

static clientInfo_t clientInfo;

typedef int (*commandHandler_t)(clientInfo_t*);

/* Command handlers */

typedef struct {
    const char *name;
    commandHandler_t handler;
} commandEntry_t;


LOCAL ICACHE_FLASH_ATTR int handleCommandLogin(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandAuth(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandPropset(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandWipe(clientInfo_t *);


commandEntry_t commandHandlers[] = {
    { "LOGIN", &handleCommandLogin },
    { "AUTH", &handleCommandAuth },
    { "PROPSET", &handleCommandPropset },
    { "WIPE", &handleCommandWipe },
    { 0, 0 }
};


void client_genRandom(unsigned char *dest)
{
    /* 10 bytes (80 bits) */
    //da39a3ee5e6b4b0d3255bfef95601890afd80709
}

LOCAL ICACHE_FLASH_ATTR void client_sendRawLine(clientInfo_t *cl)
{
    espconn_sent(&esp_conn, (uint8*)cl->tline, strlen(cl->tline));
}

LOCAL ICACHE_FLASH_ATTR void client_sendOK(clientInfo_t *cl, const char *args)
{
    os_sprintf(cl->tline,"%s OK %s\n", cl->rline, args);
    client_sendRawLine(cl);
}

LOCAL ICACHE_FLASH_ATTR void client_senderror(clientInfo_t *cl, const char *args)
{
    os_sprintf(cl->tline,"%s ERROR %s\n",cl->rline[0]?cl->rline:"?", args);
    client_sendRawLine(cl);
}


LOCAL ICACHE_FLASH_ATTR int handleCommandLogin(clientInfo_t *cl)
{
    if (cl->authenticated) {
        client_senderror(cl, "ALREADY");
        return -1;
    }
    client_sendOK(cl,"da39a3ee5e6b4b0d3255bfef95601890afd80709");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandAuth(clientInfo_t *cl)
{
    if (cl->authenticated) {
        client_senderror(cl, "ALREADY");
        return -1;
    }
    client_sendOK(cl,"WELCOME");
    cl->authenticated=1;
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandPropset(clientInfo_t *cl)
{
    if (!cl->authenticated) {
        client_senderror(cl, "UNKNONW");
        return -1;
    }
    if (cl->argc!=4) {
        client_senderror(cl, "INVALIDARGS");
        return -1;
    }

    widget_t *w = widget_find(cl->argv[1]);
    if (!w) {
        client_senderror(cl,"NOTFOUND");
        return -1;
    }

    if (widget_set_property( w, cl->argv[2], cl->argv[3])<0) {
        client_senderror(cl,"INVALIDPROP");
        return -1;
    }
    client_sendOK(cl,"PROPSET");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandWipe(clientInfo_t *cl)
{
    if (!cl->authenticated) {
        client_senderror(cl, "UNKNONW");
        return -1;
    }
    screen_destroy_all();
    client_sendOK(cl,"WIPE");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR void clientInfo_init(clientInfo_t*cl)
{
    cl->rlinepos=0;
    cl->authenticated=0;
    client_genRandom(cl->authtoken);
}


LOCAL int ICACHE_FLASH_ATTR parse_args(clientInfo_t *cl, char *start, char *end)
{
    enum { NORMAL, STRING, SPACE } state = SPACE;
    int len = end-start;
    int argc=0;

    while (len--) {
        if (argc>=8)
            return -1;

        switch(state) {
        case NORMAL:
            if (isspace(*start)) {
                *start++='\0';
                state = SPACE;
                //argc++;
                break;
            }
            if (*start=='"') {
                /* String ?... not supported */
                return -1;
                /*
                argv[argc++] = ++start;
                state = STRING;
                */
            }
            start++;
            break;
        case SPACE:
            if (isspace(*start)) {
                start++;
                break;
            }
            if (*start=='"') {
                /* String ... */
                cl->argv[argc++] = ++start;
                state = STRING;
                break;
            }
            cl->argv[argc++] = start++;
            state = NORMAL;
            break;
        case STRING:
            if (*start=='"') {
                *start++='\0';
                state = NORMAL;
            }
            /* Handle escapes here */
            start++;
            break;
        }
    }
    if (state==STRING)
        return -1;
    if (state==SPACE)
        argc--;
    return argc;
}

LOCAL ICACHE_FLASH_ATTR void client_processData(clientInfo_t *cl)
{
    cl->cmd = strchr(cl->rline, ' ');
    if (!cl->cmd) {
        client_senderror(cl,"MALFORMED");
        return;
    }
    *cl->cmd++='\0';
    cl->args = strchr(cl->cmd, ' ');
    if (cl->args) {
        *cl->args++='\0';
    }
    if (cl->args) {
        cl->argc = parse_args(cl, cl->args, &cl->rline[cl->rlinepos]);
        if (cl->argc<0) {
            client_senderror(cl,"MALFORMED");
            return;
        }
    }
    /* Find command handler */
    commandEntry_t *entry = &commandHandlers[0];
    while (entry->name) {
        if (strcmp(entry->name, cl->cmd)==0) {
            entry->handler(cl);
            break;
        }
        entry++;
    }
    if (!entry->name) {
        client_senderror(cl,"UNKNOWN");
    }
}


LOCAL ICACHE_FLASH_ATTR void client_data(clientInfo_t*cl, char *data, unsigned short length)
{
    unsigned total = (unsigned)length + cl->rlinepos;
    if (total>MAX_LINE_LEN) {
        client_senderror(cl, "TOOBIG");
        return;
    }
    /* Append to memory line */
    memcpy(&cl->rline[cl->rlinepos], data, length);
    cl->rlinepos+=length;
    cl->rline[cl->rlinepos]='\0';

    char *eol = NULL;
    do {
        /* Locate newline */
        eol = memchr(cl->rline, '\n', cl->rlinepos);
        if (eol) {
            /* Found newline. */
            *eol++='\0';
            client_processData(cl);
            /* Move back data if needed */
            unsigned remaining = eol - &cl->rline[cl->rlinepos];
            if (remaining) {
                /* No overlap, we can use memcpy */
                memcpy(cl->rline, eol, remaining);
            }
            cl->rlinepos = remaining;
        }
    } while (eol);
}

LOCAL void ICACHE_FLASH_ATTR server_recv(void *arg, char *pusrdata, unsigned short length)
{
    //struct espconn *pesp_conn = arg;
    os_printf("Server data: %d\n", length);
    client_data( &clientInfo, pusrdata, length);
}

LOCAL ICACHE_FLASH_ATTR void server_recon(void *arg, sint8 err)
{
    //struct espconn *pesp_conn = arg;
    os_printf("Server recon\n");
}

LOCAL ICACHE_FLASH_ATTR void server_conn(void *arg)
{
    struct espconn *pesp_conn = arg;
    os_printf("Server conn\n");
    clientInfo_init(&clientInfo);

    espconn_regist_time(pesp_conn, 0, 1);

}

LOCAL ICACHE_FLASH_ATTR void server_discon(void *arg)
{
    //struct espconn *pesp_conn = arg;
    os_printf("Server discon\n");
}



LOCAL void ICACHE_FLASH_ATTR server_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, server_recv);
    espconn_regist_reconcb(pesp_conn, server_recon);
    espconn_regist_connectcb(pesp_conn, server_conn);
    espconn_regist_disconcb(pesp_conn, server_discon);
}

void ICACHE_FLASH_ATTR user_server_init(uint32 port)
{
     esp_conn.type = ESPCONN_TCP;
     esp_conn.state = ESPCONN_NONE;
     esp_conn.proto.tcp = &esptcp;
     esp_conn.proto.tcp->local_port = port;
     espconn_regist_connectcb(&esp_conn, server_listen);
     espconn_regist_time(&esp_conn, 65535, 0);



#ifdef SERVER_SSL_ENABLE
     espconn_secure_accept(&esp_conn);
#else
     espconn_accept(&esp_conn);
#endif
}
