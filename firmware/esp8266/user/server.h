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
    unsigned char authtoken[20];
    unsigned  authenticated:1;
    char *argv[8];
    server_backend_t *backend;
    void *backendpvt; //     struct espconn *conn;

} clientInfo_t;

clientInfo_t *clientInfo_allocate(server_backend_t *backend, void *backendpvt);
void clientInfo_destroy(clientInfo_t *cl);
void clientInfo_reset(clientInfo_t *cl);

int client_processData(clientInfo_t *cl) WUNUSED;


// This comes from RTOS sdk.
#ifdef __linux__
#include <openssl/ssl.h>
#define SHA1_CTX SHA_CTX
#define SHA1_SIZE   20

#else
#define SHA1_SIZE   20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct
{
    uint32_t Intermediate_Hash[SHA1_SIZE/4]; /* Message Digest */
    uint32_t Length_Low;            /* Message length in bits */
    uint32_t Length_High;           /* Message length in bits */
    uint16_t Message_Block_Index;   /* Index into message block array   */
    uint8_t Message_Block[64];      /* 512-bit message blocks */
} SHA1_CTX;

void SHA1_Init(SHA1_CTX *);
void SHA1_Update(SHA1_CTX *, const uint8_t * msg, int len);
void SHA1_Final(uint8_t *digest, SHA1_CTX *);
#endif


#endif
