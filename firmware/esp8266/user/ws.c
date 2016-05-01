#include "ws.h"

#ifdef ENABLE_WEBSOCKET

#include "ets_sys.h"
#include "alloc.h"
#include "protos.h"
#include "error.h"
#include <string.h>
#include "cencode.h"
#include <ctype.h>
#include "server.h"


static const char *magicKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const unsigned magicKeyLen = 36; // Does NOT includes final NULL

LOCAL websocket_t *wssockets = NULL;
typedef int(*header_handler_fun_t)(websocket_t*s, const char *arg,int);

LOCAL int ws_header_upgrade(websocket_t*,const char*,int);
LOCAL int ws_header_connection(websocket_t*,const char*,int);
LOCAL int ws_header_secwebsocketkey(websocket_t*,const char*,int);
LOCAL void ws_client_connect( websocket_t* );


struct header_handlers_t {
    const char *header;
    header_handler_fun_t handler;
};

struct header_handlers_t header_handlers[] = {
    { "Upgrade",    &ws_header_upgrade },
    { "Connection", &ws_header_connection },
    { "Sec-WebSocket-Key", ws_header_secwebsocketkey }
};

static void os_printstr(const char *c, int len)
{
    while (len--) {
        os_printf("%c",*c);
        c++;
    }
}



LOCAL int ICACHE_FLASH_ATTR ws_parse_command(websocket_t*s, const unsigned char *data, size_t len)
{
    const unsigned char *sep,*prot;
    unsigned cmdlen;
    unsigned urilen;
    unsigned savelen = len;
    // Check command type
    sep = memchr(data,' ', len);

    if (NULL==sep)
        return EINVALIDARGUMENT;
    cmdlen = sep-data;
    os_printf("Cmd len %d\n", cmdlen);
    if (cmdlen != 3) {
        return EINVALIDARGUMENT;
    }

    if (strncmp((const char*)data,"GET",3)!=0) {
        return EINVALIDARGUMENT;
    }
    sep++;
    len -= (cmdlen+1);

    prot = memchr(sep,' ', len);
    if (NULL==prot) {
        return EINVALIDARGUMENT;
    }
    urilen = prot - sep;
    os_printf("URI len: %d\n", urilen);
    if (urilen>sizeof(s->filename)-1) {
        return EINVALIDARGUMENT;
    }

    strncpy( s->filename, (const char*)sep, urilen);

    s->state = HEADER;
    return savelen;
}

LOCAL void ICACHE_FLASH_ATTR ws_sendfilechunk(websocket_t*s)
{
    int totx = smallfsfile__size(&s->filetx) - s->filetx.seekpos;
    if (totx==0) {
        s->state = COMMAND;
        s->close=1;
        return;
    }

    if (totx>512) {
        totx=512;
    }

    smallfsfile__read(&s->filetx, s->reply, totx);
    espconn_sent(s->conn, (uint8*)s->reply, totx);

}

LOCAL void ICACHE_FLASH_ATTR ws_send_http_header(websocket_t *s,
                                                 unsigned status,
                                                 const char *statustext,
                                                 const char *content,
                                                 unsigned contentlen)
{
    if (s->reply==NULL) {
        s->reply = os_malloc(512);
    }
    int sz = os_sprintf(s->reply,"HTTP/1.0 %d %s\r\n"
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n%s",
                        status,
                        statustext,
                        contentlen,
                        content);

    espconn_sent(s->conn, (uint8*)s->reply, sz);
}

LOCAL void ICACHE_FLASH_ATTR ws_send_websocket_reply(websocket_t *s)
{
    if (s->reply==NULL) {
        s->reply = os_malloc(512);
    }
    int sz = os_sprintf(s->reply,"HTTP/1.1 101 Switching Protocols\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Accept: %s\r\n"
                        "\r\n",
                        s->wskey);

    espconn_sent(s->conn, (uint8*)s->reply, sz);
}

#define COMPARE_STR_ICASE( expected, given, len ) \
    (__builtin_strlen(expected)==len && (strncasecmp(expected,given,len)==0))

LOCAL int ICACHE_FLASH_ATTR ws_handle_file_request(websocket_t*s)
{
    // Find file.
    const char *file = s->filename;
    if (file[0] == '/')
        file++;

    if (file[0] == '\0') {
        file = "index.html";
    }

    os_printf("Requested '%s'\n", file);

    // Open it.
    if (smallfs__start()!=0) {
        os_printf("Cannot start smallfs!\n");
        ws_send_http_header(s, 400, "ISR", "ISR",3);
        return -1;
    }
    smallfs__open(smallfs__getfs(), &s->filetx, file);

    if (!smallfsfile__valid(&s->filetx)) {
        os_printf("Cannot open file!\n");
        smallfs__end(smallfs__getfs());
        ws_send_http_header(s, 404, "Not found", "Not found",9);
        return -1;
    }

    ws_send_http_header(s, 200, "OK", "",smallfsfile__size(&s->filetx));
    s->state = DATA;
    ws_sendfilechunk(s);

    return 0;
}

LOCAL int ICACHE_FLASH_ATTR ws_handle_request(websocket_t*s)
{
    os_printf("Handling request");
    os_printf("Upgrade %d, ws %d\n", s->upgrade,s->upgradews);
    os_printf("Keygen: %s\n", s->wskey);
    if (s->upgrade && s->upgradews) {
        // Switch to websocket mode.
        os_printf("Switching to websocket mode\n");
        ws_send_websocket_reply(s);
        s->state = WEBSOCKET;
        ws_client_connect(s);
        return NOERROR;
    } else {
        return ws_handle_file_request(s);
    }
}


LOCAL int ICACHE_FLASH_ATTR ws_header_upgrade(websocket_t*s,const char *data, int len)
{
    if (len!=9)
        return NOERROR;

    if (strncasecmp(data,"WebSocket",len)==0)
        s->upgradews = 1;

    return NOERROR;
}



LOCAL int ICACHE_FLASH_ATTR ws_header_option_iterate(websocket_t*s,const char *data, int len,
                                                     int(*handler)(websocket_t*s,const char *data, int len))
{
    const char *sep;
    int r = NOERROR;
    // skip any leading spaces
    os_printf("Iterating through options\n");
    do {
        while (isspace(*data) && len) {
            len--;
            data++;
        }
        if (len==0) {
            os_printf("End of data\n");
            break;
        }

        // Locate any eventual comma
        sep = memchr(data,',',len);
        if (sep) {

            os_printf("option: ");
            os_printstr(data,sep-data);
            os_printf("\n");
            r = handler(s, data, sep-data);
            if (r<0)
                break;
            len -= (sep-data);
            data = sep;
            data++;
            len--;
            while (len && isspace(*data)) {
                data++;
                len--;
            }
        } else {
            os_printf("Last option: ");
            os_printstr(data,len);
            os_printf("\n");
            r = handler(s,data,len);
            break;
        }
    } while (len!=0);
    return r;
}

LOCAL int ICACHE_FLASH_ATTR ws_header_connection_single(websocket_t*s,const char *data, int len)
{
    os_printf("Checking '");
    os_printstr(data,len);
    os_printf("'\n");
    if (COMPARE_STR_ICASE("Upgrade", data, len)) {
        s->upgrade = 1;
    }
    return NOERROR;
}

LOCAL int ICACHE_FLASH_ATTR ws_header_connection(websocket_t*s,const char *data, int len)
{
    return ws_header_option_iterate( s, data, len, &ws_header_connection_single );
}

LOCAL int ICACHE_FLASH_ATTR ws_header_secwebsocketkey(websocket_t*s,const char *data, int len)
{
    SHA1_CTX mdctx;
    unsigned char buffer[SHA1_SIZE];
    base64_encodestate bstate;
    unsigned char *rkbuf;
    int blen;

    s->wskey[0] = '\0';
    
    if (len>32) {
        return NOERROR; // Ignore
    }

    rkbuf = (unsigned char*)os_malloc(len + magicKeyLen);

    if (NULL==rkbuf)
        return ENOMEM;

    memcpy(rkbuf, data, len);
    memcpy(rkbuf+len, magicKey, magicKeyLen);

    // Compute reply key at once.
    os_printf("Hashing key: '");
    os_printstr((char*)rkbuf, len+magicKeyLen);
    os_printf("'\r\n");

    SHA1_Init(&mdctx);
    SHA1_Update(&mdctx, rkbuf, len + magicKeyLen); // dont' pass NULL terminator
    SHA1_Final(buffer, &mdctx);
   
    // Convert to Base-64
    base64_init_encodestate(&bstate);
    blen = base64_encode_block((const char*)buffer, SHA1_SIZE, (char*)s->wskey, &bstate);
    blen += base64_encode_blockend((char*)&s->wskey[blen], &bstate);
    s->wskey[blen-1]='\0'; // it has a new line, remove it.

    os_free(rkbuf);

    return NOERROR;
}

LOCAL int ICACHE_FLASH_ATTR ws_parse_header(websocket_t*s, const unsigned char *data, size_t len)
{
    const unsigned char *sep;
    size_t inlen = len;

    os_printf("Header %d\n",len);
    if (len>0) {
        // Move past eventual '\r'
        if (*data=='\r') {
            data++;
            len--;
        }
    }
    if (len==0) {
        os_printf("Headers end\n");
        ws_handle_request(s);
        return inlen;
    }
    sep = memchr(data,':', len);

    if (NULL==sep) {
        os_printf("Cannot find delimiter??? %d\n",len);
        os_printf("'%s'\n",data);
        return EINVALIDARGUMENT;
    }
    unsigned incoming_hlen = (sep-data); // Header lenght now in len.
    len -= incoming_hlen;

    sep++;
    len--;
    // Skip spaces
    while (isspace(*sep)&&len) {
        sep++;
        len--;
    }
    if (len==0)
        return EINVALIDARGUMENT;

    // "Remove" all trailing newlines/spaces if they exist
    while (len>1 && isspace(sep[len-1])) {
        len--;
    }
    struct header_handlers_t *h = &header_handlers[0];
    unsigned int i;

    os_printf("Parsing header (%d): '", incoming_hlen);
    os_printstr( (char*)data, incoming_hlen );
    os_printf("' = '");
    os_printstr( (char*)sep, len );
    os_printf("'\n");

    for (i=0;i<sizeof(header_handlers)/sizeof(header_handlers[0]);i++) {

        unsigned hlen = strlen(h->header); // TODO: optimize this

        os_printf("Len check: %d %d\n", hlen,incoming_hlen);
        if (hlen==incoming_hlen) {
            if (strncasecmp(h->header, (const char*)data, incoming_hlen)==0) {
                os_printf("Handler found");
                int r = h->handler(s, (const char*)sep, len);
                if (r<0)
                    return r;
            }
        }
        h++;
    }

    return inlen;
}

LOCAL int ws_wrapper_send(void *pvt, unsigned char *buf, size_t size)
{
    // Generate mask
    websocket_t *s = (websocket_t*)pvt;

    uint8_t hpos = 0;
    uint8_t hposneeded = 2;
    int i;

    if (size>125)
        hposneeded+=2; // 16-bit size

    os_printf("ws_wrapper_send: size %d '%s'\n",size,buf);

    for (i=size-1;i>=0;i--) {
        buf[hposneeded+i] = buf[i];
    }

    // try to optimize this, please

    buf[hpos++] = 0x81; // Text, FIN
    if (size<126) {
        buf[hpos++] = (uint8_t)size;
    } else {
        buf[hpos++] = 0x7E;
        buf[hpos++] = size<<8;
        buf[hpos++] = size;
    }
    // Transmit.
    {
        unsigned int z;
        for (z=0;z<size+hposneeded;z++) {
            os_printf("%02x ",buf[z]);
        }
        os_printf("\n");
    }
    espconn_sent( s->conn, buf, size + hposneeded );

    return NOERROR;
}

LOCAL int ws_wrapper_disconnect(void *pvt)
{
    return NOERROR;
}

LOCAL server_backend_t ws_backend = {
    &ws_wrapper_send,
    &ws_wrapper_disconnect
};


LOCAL void ICACHE_FLASH_ATTR ws_client_connect( websocket_t *s )
{
    s->client = clientInfo_allocate( &ws_backend, s);
}

LOCAL void ICACHE_FLASH_ATTR ws_datastart( websocket_t *s )
{
    clientInfo_t *cl = s->client;
    cl->rlinepos=0;
    cl->rline[cl->rlinepos]='\0';
}

LOCAL void ICACHE_FLASH_ATTR ws_append_data( websocket_t *s, unsigned char data)
{
    clientInfo_t *cl = s->client;
    if (cl) {
        cl->rline[cl->rlinepos] = data;
        cl->rlinepos++;
        cl->rline[cl->rlinepos]='\0';
    }
}

LOCAL void ICACHE_FLASH_ATTR ws_process_data( websocket_t *s )
{
    if (client_processData(s->client)!=NOERROR) {
        clientInfo_destroy(s->client);
        s->client = NULL;
    }
}

LOCAL int ICACHE_FLASH_ATTR ws_parse_chunk( websocket_t *s, const unsigned char *data, size_t len)
{
    int leftover = len;
    int r;
    const unsigned char *eol;

    do {
        eol = memchr(data, '\n', len);
        if (NULL==eol) {
            leftover = 0; // Nothing processed.
            break;
        }
        switch (s->state) {
        case COMMAND:
            r = ws_parse_command(s,data,eol-data);
            break;
        case HEADER:
            r = ws_parse_header(s,data,eol-data);
            break;
        default:
            r = EINVALIDARGUMENT;
            break;
        }
        if (r<0)
            break;

        r++; // Move past newline

        leftover -= r;
        len -= r;
        data += r;

        os_printf("Leftover: %d\n", leftover);
    } while (1);

    return leftover;
}

LOCAL int ICACHE_FLASH_ATTR handle_websocket_data(websocket_t *s,const unsigned char *data, size_t len)
{
    uint8_t realdata;
    os_printf("WS data size: %d\n",len);

    {
        unsigned int i;
        for (i=0;i<len;i++) {
            os_printf("%02x ",data[i]);
        }
        os_printf("\n");
    }

    while (len) {
        //os_printf("Loop len %d state %d data 0x%02x\n",len,s->wsstate, *data);
        switch (s->wsstate) {
        case WSCMD1:
            s->hdr1 = *data;
            s->wsstate = WSCMD2;
            s->size = 0;
            break;
        case WSCMD2:
            s->hdr2 = *data;

            if ((s->hdr2 & 0x7f) == 126) {
                s->sizelen = 2;
                s->wsstate = WSLEN;
                break;
            }

            if ((s->hdr2 & 0x7f) == 127) {
                s->sizelen = 8;
                s->wsstate = WSLEN;
                break;
            }

            s->size += s->hdr2 & 0x7F;

            if (s->hdr2 & WS_HDR2_MASKBIT) {
                s->sizelen = 4;
                s->wsstate = WSMASK;
            } else {
                ws_datastart(s);
                s->wsstate = WSDATA; // Unlikely
            }

            break;
        case WSLEN:
            s->size <<=8;
            s->size += (uint64_t)(*data);
            s->sizelen--;
            if (s->sizelen==0) {
                if (s->hdr2 & WS_HDR2_MASKBIT) {
                    s->sizelen = 4;
                    s->wsstate = WSMASK;
                } else {
                    ws_datastart(s);
                    s->wsstate = WSDATA;
                }
            }
            break;
        case WSMASK:
            s->mask[ 4- s->sizelen ] = *data;
            s->sizelen--;
            if (s->sizelen==0) {
                s->maskidx = 0;
                ws_datastart(s);
                s->wsstate = WSDATA;
            }
            break;

        case WSDATA:
            realdata = *data;

            if (s->hdr2 & WS_HDR2_MASKBIT)
                realdata ^= s->mask[s->maskidx];
            s->maskidx++;
            s->maskidx&=3;
            ws_append_data(s, realdata);
            s->size --;
            if (s->size==0) {
                ws_process_data(s);
                s->wsstate = WSCMD1;
            }
            break;
        }
        data++;
        len--;

    }

    return 0;
}

LOCAL int ICACHE_FLASH_ATTR ws_data_impl( websocket_t *s, const unsigned char *data, size_t len)
{
    int datap;
    const unsigned char *source = data;

    os_printf("Data conn len %d\n",len);
    if (len==0)
        return 0;

    switch (s->state) {
    case COMMAND:
    case HEADER:
        if (s->qlen) {
            // We already had data. Need to append.
            s->qdata = os_realloc(s->qdata, s->qlen + len);
            if (s->qdata==NULL)
                return ENOMEM;
            memcpy( &s->qdata[s->qlen], data, len);
            source = s->qdata;
            len += s->qlen;
            s->qlen = len;
        }

        datap = ws_parse_chunk(s, source, len);
        if (datap<0) {
            return datap;
        }
        // is there any data remaining ?
        unsigned leftover = len - datap;
        if (leftover) {
            s->qdata = os_realloc(s->qdata, leftover);
            if (s->qdata==NULL)
                return ENOMEM;
            memmove( s->qdata, &source[ (len-leftover) ], leftover);
        }
        s->qlen = leftover;

        break;
    case DATA:
        break;
    case WEBSOCKET:
        os_printf("WS data\n");
        if (handle_websocket_data(s,data,len)<0)
            s->close=1;
        break;
    default:
        break;
    }
    return 0;
}



LOCAL websocket_t * ICACHE_FLASH_ATTR find_by_conn(struct espconn *conn)
{
    websocket_t *s = wssockets;
    while (s) {
        if (s->conn == conn)
            break;
        s=s->next;
    }
    return s;
}

int ICACHE_FLASH_ATTR ws_data( struct espconn*conn, const unsigned char *data, size_t len)
{
    websocket_t *s = find_by_conn(conn);
    if (NULL==s)
        return -1;

    int r = ws_data_impl(s,data,len);
    os_printf("Data result: %d, close %d\n",r,s->close);
    if (r<0 || s->close) {
        // Close socket.
        espconn_disconnect(s->conn);
    }
    return r;
}

void ICACHE_FLASH_ATTR ws_connect( struct espconn *conn )
{
    websocket_t *s = os_calloc(sizeof(websocket_t), 1);
    s->next = wssockets;
    s->conn = conn;
    wssockets = s;
}

void ICACHE_FLASH_ATTR ws_datasent( struct espconn *conn )
{
    websocket_t *s = find_by_conn(conn);
    if (s) {
        if (s->state == DATA)
            ws_sendfilechunk(s);
    }
}

LOCAL void ICACHE_FLASH_ATTR ws_destroy(websocket_t *s)
{
    if (s->reply)
        os_free(s->reply);
    if (s->client) {
        clientInfo_destroy(s->client);
    }
    os_free(s);
}

void ICACHE_FLASH_ATTR ws_disconnect( struct espconn *conn )
{
    websocket_t *prev = NULL;
    websocket_t *s = wssockets;
    os_printf("Disconnect called.\n");
    while (s) {
        if (s->conn == conn)
            break;
        prev = s;
        s = s->next;
    }
    if (NULL==s)
        return;

    //ws_data_impl(s, NULL, 0); // Notify we closed.

    if (prev) {
        prev->next = s->next;
    } else {
        wssockets = s->next;
    }
    ws_destroy(s);
}

#endif
