#include <stdio.h>
#include <openssl/ssl.h>
#include <ctype.h>
#include <inttypes.h>

#define SHA1_CTX SHA_CTX
#define SHA1_SIZE   20

#define LOCAL static
#define ICACHE_FLASH_ATTR

LOCAL char nibble2ascii(char nibble)
{
    nibble&=0xf;
    if (nibble>9) {
        return 'A'+(nibble-10);
    }
    return '0'+nibble;
}

LOCAL char *puthexbyte(unsigned char byte, char *target)
{
    *target++ = nibble2ascii(byte>>4);
    *target++ = nibble2ascii(byte);
    return target;
}

LOCAL int ICACHE_FLASH_ATTR ascii2nibble(char in)
{
    in = tolower(in);
    if (in>='0' && in<='9') {
        return in - '0';
    }
    if (in>='a' && in<='f') {
        return in - 'a' + 10;
    }
    return -1;
}

LOCAL int ICACHE_FLASH_ATTR parsehex(const char *input, unsigned char*dest, size_t maxsize)
{
    int i,j;
    int r=0;
    do {
        if (*input=='\0')
            break;
        i = ascii2nibble(*input++);
        if (i<0)
            return -1;
        j = ascii2nibble(*input++);
        if (j<0)
            return -1;
        *dest++ = (i<<4) + j;
        r++, maxsize--;
    } while (maxsize);

    if (*input!='\0')
        return -1;

    return r;
}


LOCAL const char *ICACHE_FLASH_ATTR getPassword()
{
    return "admin";
}

LOCAL int ICACHE_FLASH_ATTR getPasswordLen()
{
    return 5;
}

int main(int argc, char **argv)
{
    unsigned char key[SHA1_SIZE];
    unsigned char computed[SHA1_SIZE];

    SHA1_CTX ctx;

    if (argc<2)
        return -1;

    parsehex(argv[1], key, SHA1_SIZE);

    // Verify.
    SHA1_Init(&ctx);

    SHA1_Update( &ctx, key, 20);
    SHA1_Update( &ctx, (const uint8_t*)getPassword(), getPasswordLen());
    SHA1_Final( &computed[0], &ctx );
    unsigned i;
    char *dest;
    char tok[41];
    dest = tok;

    for (i=0; i<20; i++) {
        dest = puthexbyte(computed[i], dest);
    }
    *dest='\0';
    printf("AUTH %s\n",tok);
    return 0;
}
