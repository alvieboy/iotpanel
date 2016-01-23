#include "serdes.h"
#include "alloc.h"
#include "protos.h"
#include <string.h>

#define     DEBUGSERIALIZE(x...) os_printf(x)

int ICACHE_FLASH_ATTR serialize_string(serializer_t *f, const char *str)
{
    const char nullstr[] = "";
    if (NULL==str)
        str = nullstr;

    uint16_t len = strlen(str);
    int r;
    r =serialize_uint16(f, len);
    if (r==0 && len>0) {
        r = serialize_bytearray(f, (const unsigned char*)str, len);
    }
    return r;
}

int ICACHE_FLASH_ATTR serialize_bytearray(serializer_t *f, const unsigned char *ptr, unsigned size)
{
    return f->write(f, ptr, size);
}

int ICACHE_FLASH_ATTR serialize_long(serializer_t *f, long i)
{
    unsigned char buf[5];
    long ic=i;
    int need=0,ret = 0;

    if (i>=0) {

        do {
            buf[need++] = ic & 127;
            ic>>=7;
        } while (ic!=0);
    } else {
        do {
            buf[need++] = ic & 127;
            ic>>=7;
        } while (ic!=-1 || ((buf[need-1]&0x40)==0));
    }

    if (i>=0) {
        if (buf[need-1] & 0x40) {
            // Need to inject
            buf[need++] = 0;
        }
    }

    while (need) {
        need--;
        uint8_t val = buf[need];
        if (need!=0) {
            val|=0x80;
        }
        DEBUGSERIALIZE("[%02x] ", val);
        f->write( f, &val, sizeof(val) );
    }
    return ret;
}

int ICACHE_FLASH_ATTR serialize_int8(serializer_t *f, int8_t i)
{
    return serialize_long(f,(long)i);
}

int ICACHE_FLASH_ATTR serialize_uint8(serializer_t *f, uint8_t i)
{
    return serialize_long(f,(long)i);
}

int ICACHE_FLASH_ATTR serialize_int16(serializer_t *f, int16_t i)
{
    return serialize_long(f,(long)i);
}

int ICACHE_FLASH_ATTR serialize_uint16(serializer_t *f, uint16_t i)
{
    return serialize_long(f,(long)i);
}

int ICACHE_FLASH_ATTR serialize_int32(serializer_t *f, int32_t i)
{
    return serialize_long(f,(long)i);
}

int ICACHE_FLASH_ATTR serialize_uint32(serializer_t *f, uint32_t i)
{
    return serialize_long(f,(long)i);
}

char *ICACHE_FLASH_ATTR deserialize_string_alloc(serializer_t *f)
{
    uint16_t len = 0;
    char *r = NULL;
    do {
        if (deserialize_uint16(f,&len)<0)
            break;
        r = (char*)os_malloc(len+1);
        if (r) {
            if (deserialize_bytearray(f, (unsigned char*)r, len)<0) {
                os_free(r);
                r=NULL;
            }
            if (r) {
                r[len]='\0';
            }
        }
    } while (0);
    return r;

}


int ICACHE_FLASH_ATTR deserialize_string(serializer_t *f, char *dest, unsigned *size, unsigned maxsize)
{
    uint16_t len = 0;
    int r;
    do {
        r = deserialize_uint16(f,&len);
        if (r<0)
            break;
        if (len>maxsize) {
            os_printf("String too large: %d maxsize %d\n", (unsigned)len, maxsize);
            r = -1;
            break;
        }

        if (NULL!=size) {
            *size = len;
        }
        r = deserialize_bytearray(f, (unsigned char*)dest, len);
        if (r>=0) {
            dest[len]='\0';
        }
    } while (0);
    return r;
}

int ICACHE_FLASH_ATTR deserialize_bytearray(serializer_t *f, unsigned char *dest, unsigned size)
{
    int r = f->read(f, dest, size);
    return r;
}

int ICACHE_FLASH_ATTR deserialize_long(serializer_t *f, long *dest)
{
    int first = 1;
    long val = 0;
    uint8_t v;
    int r;

    do {
        r = f->read(f, &v, sizeof(v));
        if (r!=0)
            return -1;

        if (first && (v &0x40)) {
            val=-1;
        }
        val<<=7;
        val|=v&0x7f;
        first=0;
    } while (v&0x80);

    *dest = val;
    return 0;
}

int ICACHE_FLASH_ATTR deserialize_int8(serializer_t *f, int8_t *i)
{
    long val;
    int r;
    r = deserialize_long(f, &val);
    if (r==0) {
        *i = val & 0xff;
    }
    return r;
}

int ICACHE_FLASH_ATTR deserialize_uint8(serializer_t *f, uint8_t *i)
{
    return deserialize_int8(f,(int8_t*)i);
}

int ICACHE_FLASH_ATTR deserialize_int16(serializer_t *f, int16_t *i)
{
    long val;
    int r;
    r = deserialize_long(f, &val);
    if (r==0) {
        *i = val & 0xffff;
    }
    return r;
}

int ICACHE_FLASH_ATTR deserialize_uint16(serializer_t *f, uint16_t *i)
{
    return deserialize_int16(f,(int16_t*)i);
}

int ICACHE_FLASH_ATTR deserialize_int32(serializer_t *f, int32_t *i)
{
    long val;
    int r;
    r = deserialize_long(f, &val);
    if (r==0) {
        *i = val & 0xffffffff;
    }
    return r;
}

int ICACHE_FLASH_ATTR deserialize_uint32(serializer_t *f, uint32_t *i)
{
    return deserialize_int32(f,(int32_t*)i);
}

