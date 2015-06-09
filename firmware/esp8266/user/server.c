#ifdef __linux__
#else

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include <stdio.h>

#endif

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
    unsigned char authtoken[10];
    unsigned  authenticated:1;

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


commandEntry_t commandHandlers[] = {
    { "LOGIN", &handleCommandLogin },
    { "AUTH", &handleCommandAuth },
    { 0, 0 }
};


void client_genRandom(unsigned char *dest)
{
    /* 10 bytes (80 bits) */
    //da39a3ee5e6b4b0d3255bfef95601890afd80709
}

LOCAL ICACHE_FLASH_ATTR client_sendRawLine(clientInfo_t *cl)
{
    espconn_sent(&esp_conn, cl->tline, strlen(cl->tline));
}

LOCAL ICACHE_FLASH_ATTR client_sendOK(clientInfo_t *cl, const char *args)
{
    os_sprintf(cl->tline,"%s OK %s", cl->rline, args);
    client_sendRawLine(cl);
}

LOCAL ICACHE_FLASH_ATTR client_senderror(clientInfo_t *cl, const char *args)
{
    os_sprintf(cl->tline,"? ERROR %s", cl->rline, args);
    client_sendRawLine(cl);
}


LOCAL ICACHE_FLASH_ATTR int handleCommandLogin(clientInfo_t *cl)
{
    if (cl->authenticated) {
        client_senderror(cl->rline, "ALREADY");
        return;
    }
    client_sendOK(cl,"da39a3ee5e6b4b0d3255bfef95601890afd80709");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandAuth(clientInfo_t *cl)
{
    if (cl->authenticated) {
        client_senderror(cl->rline, "ALREADY");
        return;
    }
    client_sendOK(cl,"");
    cl->authenticated=1;
    return 0;
}



LOCAL ICACHE_FLASH_ATTR void clientInfo_init(clientInfo_t*cl)
{
    cl->rlinepos=0;
    cl->authenticated=0;
    client_genRandom(cl->authtoken);
}

LOCAL ICACHE_FLASH_ATTR void client_processMessage(clientInfo_t *cl)
{
}

LOCAL ICACHE_FLASH_ATTR void client_processData(clientInfo_t *cl)
{
    cl->cmd = strchr(cl->rline, ' ');
    if (!cl->cmd) {
        client_senderror("?","MALFORMED");
        return;
    }
    *cl->cmd++='\0';
    cl->args = strchr(cl->cmd, ' ');
    if (cl->args) {
        *cl->args++='\0';
    }

    /* Find command handler */


}


LOCAL ICACHE_FLASH_ATTR void client_data(clientInfo_t*cl, char *data, unsigned short length)
{
    unsigned total = (unsigned)length + cl->rlinepos;
    if (total>MAX_LINE_LEN) {
        client_senderror("?", "TOOBIG");
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
    struct espconn *pesp_conn = arg;
    os_printf("Server data: %d\n", length);
    client_data( &clientInfo, pusrdata, length);
}

LOCAL ICACHE_FLASH_ATTR void server_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;
    os_printf("Server recon\n");
}

LOCAL ICACHE_FLASH_ATTR void server_conn(void *arg)
{
    struct espconn *pesp_conn = arg;
    os_printf("Server conn\n");
    clientInfo_init(&clientInfo);
}

LOCAL ICACHE_FLASH_ATTR void server_discon(void *arg)
{
    struct espconn *pesp_conn = arg;
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

#ifdef SERVER_SSL_ENABLE
     espconn_secure_accept(&esp_conn);
#else
     espconn_accept(&esp_conn);
#endif
}
