#ifndef __SERVER_H__
#define __SERVER_H__

#include "os_type.h"
#include "protos.h"

#define MAX_LINE_LEN 1024

typedef struct server_backend_t {
    int (*send)(void *, unsigned char *buf, size_t size);
    int (*disconnect)(void*);
} server_backend_t;

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
    server_backend_t *backend;
    void *backendpvt; //     struct espconn *conn;

} clientInfo_t;

clientInfo_t *clientInfo_allocate(server_backend_t *backend, void *backendpvt);
void clientInfo_destroy(clientInfo_t *cl);
void clientInfo_reset(clientInfo_t *cl);

int client_processData(clientInfo_t *cl) WUNUSED;


#endif
