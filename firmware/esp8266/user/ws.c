#include "ws.h"

#ifdef ENABLE_WEBSOCKET

#include "ets_sys.h"
#include "alloc.h"
#include "protos.h"
#include "error.h"
#include <string.h>

LOCAL websocket_t *wssockets = NULL;

LOCAL int ws_parse_command(websocket_t*s, const unsigned char *data, size_t len)
{
    const unsigned char *sep;
    // Check command type
    sep = memchr(data,' ', len);

    if (NULL==sep)
        return EINVALIDARGUMENT;

    os_printf("Cmd len %d\n", sep-data);
    s->state = HEADER;
    return len;
}

LOCAL int ws_handle_request(websocket_t*s)
{
    const char reply[] = "HTTP/1.0 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "Content-Length: 22\r\n"
        "\r\n"
        "The item was not found";
    espconn_sent(s->conn, (uint8*)reply, sizeof(reply)-1);
    s->close = 1;
    return 0;
}

LOCAL int ws_parse_header(websocket_t*s, const unsigned char *data, size_t len)
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

    // TODO: parse headers.

    return inlen;
}

LOCAL int ws_parse_chunk( websocket_t *s, const unsigned char *data, size_t len)
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

LOCAL int ws_data_impl( websocket_t *s, const unsigned char *data, size_t len)
{
    int datap;
    const unsigned char *source = data;

    os_printf("Data conn. '%s'\n",data);
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
    case WEBSOCKET:
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

int ws_data( struct espconn*conn, const unsigned char *data, size_t len)
{
    websocket_t *s = find_by_conn(conn);
    if (NULL==s)
        return -1;

    int r = ws_data_impl(s,data,len);
    if (r<0 || s->close) {
        // Close socket.
        espconn_disconnect(s->conn);
    }
    return r;
}

void ws_connect( struct espconn *conn )
{
    websocket_t *s = os_calloc(sizeof(websocket_t), 1);
    s->next = wssockets;
    s->conn = conn;
    wssockets = s;
}

void ws_datasent( struct espconn *conn )
{
}

void ws_disconnect( struct espconn *conn )
{
    websocket_t *prev = NULL;
    websocket_t *s = wssockets;
    os_printf("Disconnect\n");
    while (s) {
        if (s->conn == conn)
            break;
        prev = s;
        s = s->next;
    }
    if (NULL==s)
        return;

    ws_data_impl(s, NULL, 0); // Notify we closed.

    if (prev) {
        prev->next = s->next;
    } else {
        wssockets = s->next;
    }
    os_free(s);

}

#endif
