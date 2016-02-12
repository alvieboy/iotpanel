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
#include "debug.h"
#include "widget_registry.h"
#include "protos.h"
#include "schedule.h"
#include <stdlib.h>
#include "upgrade.h"
#include "cdecode.h"
#include "clock.h"
#include "flash_serializer.h"
#include "user_interface.h"
#include "alloc.h"

LOCAL esp_tcp esptcp;
LOCAL struct espconn esp_conn;
char currentFw[32] = {0};
static unsigned char *otachunk;

extern void setBlanking(int);

#define MAX_LINE_LEN 1024

typedef struct {
    char rline[MAX_LINE_LEN+1];
    char tline[128+1];
    unsigned  rlinepos;
    // Command placeholders.
    char *cmd;
    char *args;
    int argc;
    unsigned char authtoken[10];
    unsigned  authenticated:1;
    char *argv[8];
    struct espconn *conn;
} clientInfo_t;

static clientInfo_t clientInfo;

typedef int (*commandHandler_t)(clientInfo_t*);

/* Command handlers */

typedef struct {
    const char *name;
    commandHandler_t handler;
    unsigned needauth:1;
    const char *help;
} commandEntry_t;


LOCAL ICACHE_FLASH_ATTR int handleCommandLogin(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandAuth(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandPropset(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandWipe(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandNewScreen(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandSelect(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandAdd(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandHelp(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandFwGet(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandFwSet(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandNewSchedule(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandAddSchedule(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandSchedule(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandSetTime(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandClone(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandLogout(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandBlank(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandOTA(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandSave(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandReset(clientInfo_t *);
LOCAL ICACHE_FLASH_ATTR int handleCommandInfo(clientInfo_t *);

commandEntry_t commandHandlers[] = {
    { "HELP",    &handleCommandHelp, 0, "[<commandname>]" },
    { "LOGIN",   &handleCommandLogin, 0 ,"<username>"},
    { "AUTH",    &handleCommandAuth, 0, "<authtoken>" },
    { "PROPSET", &handleCommandPropset, 1, "<widgetname> <propertyname> <value>" },
    { "WIPE",    &handleCommandWipe, 1 ,""},
    { "NEWSCREEN", &handleCommandNewScreen, 1,"<screenname>" },
    { "SELECT",    &handleCommandSelect, 1,"<screenname>" },
    { "ADD",    &handleCommandAdd, 1,"<screenname> <widgetclass> <widgetname> <x> <y>" },
    { "FWGET",  &handleCommandFwGet, 1, "" },
    { "FWSET",  &handleCommandFwSet, 1, "" },
    { "NEWSCHEDULE",  &handleCommandNewSchedule, 1, "" },
    { "ADDSCHEDULE",  &handleCommandAddSchedule, 1, "(SELECT|WAIT) <args>" },
    { "SCHEDULE",  &handleCommandSchedule, 1, "(START|STOP)" },
    { "SETTIME",  &handleCommandSetTime, 1, "<seconds>" },
    { "CLONE",  &handleCommandClone, 1, "<widgetname> <screenname> <x> <y>" },
    { "BLANK",  &handleCommandBlank, 1, "<blank>" },
    { "OTA",  &handleCommandOTA, 1, "(opt)" },
    { "SAVE",  &handleCommandSave, 1, "" },
    { "LOGOUT",   &handleCommandLogout, 0 ,""},
    { "RESET",   &handleCommandReset, 1 ,""},
    { "INFO",   &handleCommandInfo, 1 ,""},
    { 0, 0, 1, NULL }
};


LOCAL void ICACHE_FLASH_ATTR client_genRandom(unsigned char *dest)
{
    int i, v;
    /* 10 bytes (80 bits) */
    srand(system_get_time());
    for (i=0;i<10;i++) {
        v = rand();
        *dest++ = v&0xff;
    }
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

LOCAL ICACHE_FLASH_ATTR int handleCommandHelp(clientInfo_t *cl)
{
    if (cl->argc>1) {
        client_senderror(cl, "INVALIDARGS");
        return -1;
    }
    client_senderror(cl, "INVALIDARGS");
    return -1;
#if 0
    if (cl->argv==0) {
        //os_sprintf(cl->tline,"%s ERROR %s\n",cl->rline[0]?cl->rline:"?", args);
        client_sendRawLine(cl);
    } else {
    }
    return 0;
#endif
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
    if (cl->argc!=3) {
        client_senderror(cl, "INVALIDARGS");
        return -1;
    }

    widget_t *w = widget_find(cl->argv[0]);
    if (!w) {
        client_senderror(cl,"NOTFOUND");
        return -1;
    }

    if (widget_set_property( w, cl->argv[1], cl->argv[2])<0) {
        client_senderror(cl,"INVALIDPROP");
        return -1;
    }
    client_sendOK(cl,"PROPSET");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandWipe(clientInfo_t *cl)
{
    screen_destroy_all();
    client_sendOK(cl,"WIPE");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandNewScreen(clientInfo_t *cl)
{
    screen_t *s = screen_create(cl->argv[0]);
    if (s) {
        client_sendOK(cl,"NEWSCREEN");
    } else {
        client_senderror(cl,"TOOMANY");
    }
    return s!=NULL;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandSelect(clientInfo_t *cl)
{
    if (cl->argc!=1) {
        client_senderror(cl,"INVALIDARGS");
    }
    screen_t *s = screen_find(cl->argv[0]);
    if (s) {
        screen_select(s);
        client_sendOK(cl,"SELECT");
    } else {
        client_senderror(cl,"NOTFOUND");
    }
    return s!=NULL;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandAdd(clientInfo_t *cl)
{
    int x, y;
    char *end;

    if (cl->argc<5) {
        client_senderror(cl,"INVALIDARGS");
    }
    screen_t *s = screen_find(cl->argv[0]);
    if (!s) {
        client_senderror(cl,"NOTFOUND");
        return -1;
    }

    const widgetdef_t *c = widgetdef_find(cl->argv[1]);
    if (!c) {
        client_senderror(cl,"INVALID");
        return -1;
    }

    widget_t *w = widget_find(cl->argv[2]);
    if (w) {
        client_senderror(cl,"ALREADY");
        return -1;
    }

    x = (int)strtol(cl->argv[3],&end,10);
    if (*end !='\0') {
        client_senderror(cl,"INVALID");
        return -1;
    }

    y = (int)strtol(cl->argv[4],&end,10);
    if (*end !='\0') {
        client_senderror(cl,"INVALID");
        return -1;
    }

    /* Ok, create it. */

    w = widget_create(cl->argv[1], cl->argv[2]);
    if (!w) {
        client_senderror(cl,"INVALID");
        return -1;
    }

    screen_add_widget(s, w, x, y);
    client_sendOK(cl,"ADD");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandBlank(clientInfo_t *cl)
{
    int b;
    char *end;

    if (cl->argc<1) {
        client_senderror(cl,"INVALIDARGS");
    }
    b = (int)strtol(cl->argv[0],&end,10);
    if (*end !='\0') {
        client_senderror(cl,"INVALID");
        return -1;
    }
    setBlanking(b);
    client_sendOK(cl,"ADD");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandClone(clientInfo_t *cl)
{
    int x, y;
    char *end;

    if (cl->argc<4) {
        client_senderror(cl,"INVALIDARGS");
    }
    screen_t *s = screen_find(cl->argv[1]);
    if (!s) {
        client_senderror(cl,"NOTFOUND");
        return -1;
    }

    widget_t *w = widget_find(cl->argv[0]);
    if (!w) {
        client_senderror(cl,"NOTFOUND");
        return -1;
    }

    x = (int)strtol(cl->argv[2],&end,10);
    if (*end !='\0') {
        client_senderror(cl,"INVALID");
        return -1;
    }

    y = (int)strtol(cl->argv[3],&end,10);
    if (*end !='\0') {
        client_senderror(cl,"INVALID");
        return -1;
    }

    /* Ok, create it. */
    screen_add_widget(s, w, x, y);
    client_sendOK(cl,"CLONE");
    return 0;
}


LOCAL ICACHE_FLASH_ATTR int handleCommandFwGet(clientInfo_t *cl)
{
    client_sendOK(cl,currentFw);
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandFwSet(clientInfo_t *cl)
{
    if (cl->argc!=1) {
        client_senderror(cl,"INVALIDARGS");
        return -1;
    }
    strncpy( currentFw, cl->argv[0], sizeof(currentFw));
    client_sendOK(cl,"FWSET");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandNewSchedule(clientInfo_t *cl)
{
    schedule_reset();
    client_sendOK(cl,"NEWSCHEDULE");

    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandAddSchedule(clientInfo_t *cl)
{
    if (cl->argc<2) {
        client_senderror(cl,"INVALID");
        return -1;
    }
    if (strcmp(cl->argv[0], "SELECT")==0) {
        schedule_append(SCHEDULE_SELECT, cl->argv[1]);
    } else if (strcmp(cl->argv[0], "WAIT")==0) {
        schedule_append(SCHEDULE_WAIT, cl->argv[1]);
    }
    client_sendOK(cl,"ADDSCHEDULE");

    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandSchedule(clientInfo_t *cl)
{
    if (cl->argc!=1) {
        client_senderror(cl,"INVALID");
        return -1;
    }
    if (strcmp(cl->argv[0], "START")==0) {
        schedule_start();
    } else if (strcmp(cl->argv[0], "STOP")==0) {
        schedule_stop();
    } else {
        client_senderror(cl,"INVALID");
        return -1;
    }
    client_sendOK(cl,"SCHEDULE");

    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandSetTime(clientInfo_t *cl)
{
    uint32 s;
    char *end = NULL;

    if (cl->argc!=1) {
        client_senderror(cl,"INVALID");
        return -1;
    }

    s = (unsigned int)strtol(cl->argv[0],&end,10);
    if (end && *end=='\0') {
        time_set_seconds(s);
        client_sendOK(cl,"SETTIME");
        return 0;
    } else {
        return -1;
    }

}

LOCAL ICACHE_FLASH_ATTR int handleCommandLogout(clientInfo_t *cl)
{
    client_sendOK(cl,"LOGOUT");
    cl->authenticated=0;
    return -2;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandOTA(clientInfo_t *cl)
{
    char otainfo[128];
    char *end = NULL;

    if (cl->argc<1) {
        client_senderror(cl,"INVALID");
        return -1;
    }

    if (strcmp(cl->argv[0],"STATUS")==0) {
        os_sprintf(otainfo,"STATUS 0x%08x free 0x%08x",
                   get_last_firmware_status(),
                   get_free_flash()
                  );

        client_sendOK(cl, otainfo);
        return 0;
    } else if (strcmp(cl->argv[0],"START")==0) {
        ota_initialize();
    } else if (strcmp(cl->argv[0],"CHUNK")==0) {
        if (cl->argc<3) {
            client_senderror(cl,"INVALID");
            return -1;
        } else {
            // Parse address and size
            uint32 start = (unsigned int)strtol(cl->argv[1],&end,10);
            uint32 size;
            if (end && *end=='\0') {
                // Parse address and size
                size = (unsigned int)strtol(cl->argv[2],&end,10);
                if (end && *end=='\0') {
                    if (ota_set_chunk(start,size)<0) {
                        client_senderror(cl,"OTA CHUNKERROR");
                        return -1;
                  }
                } else {
                    client_senderror(cl,"MALFORMED");
                    return -1;
                }
            } else {
                client_senderror(cl,"MALFORMED");
                return -1;
            }
        }
    } else if (strcmp(cl->argv[0],"DATA")==0) {

        base64_decodestate b64state;

        if ( cl->argc<2) {
            client_senderror(cl,"INVALID");
            return -1;
        }
        base64_init_decodestate(&b64state);
        // Attempt to parse it

        if (otachunk==NULL) {
            otachunk = (unsigned char *)os_malloc(512);
        }

        int r = base64_decode_block((char*)cl->argv[1],
                                    strlen(cl->argv[1]),
                                    (char*)otachunk,
                                    &b64state);
        if (r!= 512) {
            os_printf("Invalid len of %d\n", r);
            client_senderror(cl,"INVALIDLEN");
            return -1;
        }


        if (ota_program_chunk(otachunk)<0) {
            client_senderror(cl,"OTA PROGRAM");
            return -1;
        }
    } else if (strcmp(cl->argv[0],"FINALISE")==0) {
        client_sendOK(cl,"OTA");
        espconn_disconnect(cl->conn);
        ota_finalize();
    } else {
        client_senderror(cl,"INVALID");
        return -1;
    }

    client_sendOK(cl,"OTA");

    return 0;
}


LOCAL ICACHE_FLASH_ATTR int handleCommandSave(clientInfo_t *cl)
{
    serialize_all(&flash_serializer);
    client_sendOK(cl,"SAVE");
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandReset(clientInfo_t *cl)
{
    client_sendOK(cl,"RESET");
    system_restart();
    return 0;
}

LOCAL ICACHE_FLASH_ATTR int handleCommandInfo(clientInfo_t *cl)
{
    char info[64];
    uint32 memfree = system_get_free_heap_size();
    os_sprintf(info,"%u", memfree);
    client_sendOK(cl,info);
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
    int argc=0;

    end--;

    int len = end-start;

    while (len--) {
        if (argc>=8)
            return -1;
        if ( (*start == 0x0d ) || (*start == 0x0a)) {
            *start++='\0';
            len=0;
        }
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
    cl->argc = 0;
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
            if (entry->needauth && !cl->authenticated) {
                client_senderror(cl,"UNKNONW");
            } else {
                if (entry->handler(cl)==-2) {
                    os_printf("Destroying connection\n");
                    espconn_disconnect(cl->conn);
                    espconn_delete(cl->conn);
                    cl->conn = NULL;
                }
            }
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
    clientInfo.conn = arg;//struct espconn *pesp_conn = arg;
    DEBUG("Server data: %d\n", length);
    client_data( &clientInfo, pusrdata, length);
}

LOCAL ICACHE_FLASH_ATTR void server_recon(void *arg, sint8 err)
{
    //struct espconn *pesp_conn = arg;
    DEBUG("Server recon\n");
}

LOCAL ICACHE_FLASH_ATTR void server_conn(void *arg)
{
    struct espconn *pesp_conn = arg;
    DEBUG("Server conn\n");
    clientInfo_init(&clientInfo);
    clientInfo.conn = pesp_conn;

    espconn_regist_time(pesp_conn, 7200, 1);

}

LOCAL ICACHE_FLASH_ATTR void server_discon(void *arg)
{
    //struct espconn *pesp_conn = arg;
    DEBUG("Server discon\n");
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
     espconn_regist_time(&esp_conn, 7200, 0);



#ifdef SERVER_SSL_ENABLE
     espconn_secure_accept(&esp_conn);
#else
     espconn_accept(&esp_conn);
#endif
}
