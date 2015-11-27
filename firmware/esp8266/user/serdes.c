#include "serdes.h"

int ICACHE_FLASH_ATTR serialize_string(serializer_t *f, const char *str)
{
    uint16_t len = strlen(str);
    int r;
    r =serialize_uint16(f, len);
    if (r==0) {
        r = serialize_bytearray(f, (const unsigned char*)str, len);
    }
    return r;
}

int ICACHE_FLASH_ATTR serialize_bytearray(serializer_t *f, const unsigned char *ptr, unsigned size)
{
    return f->write(f, ptr, size);
}

int ICACHE_FLASH_ATTR serialize_int8(serializer_t *f, int8_t i)
{
    return serialize_uint8(f,(uint8_t)i);
}

int ICACHE_FLASH_ATTR serialize_uint8(serializer_t *f, uint8_t i)
{
    uint8_t data;
    data = i;
    return f->write(f, &data, sizeof(data) );
}

int ICACHE_FLASH_ATTR serialize_int16(serializer_t *f, int16_t i)
{
    return serialize_uint16(f,(uint16_t)i);
}

int ICACHE_FLASH_ATTR serialize_uint16(serializer_t *f, uint16_t i)
{
    uint8_t data[2];
    data[0] = i>>8;
    data[1] = i&0xff;
    return f->write(f, &data[0], sizeof(data) );
}

int ICACHE_FLASH_ATTR serialize_int32(serializer_t *f, int32_t i)
{
    return serialize_uint32(f,(uint32_t)i);
}

int ICACHE_FLASH_ATTR serialize_uint32(serializer_t *f, uint32_t i)
{
    uint8_t data[4];
    data[0] = i>>24;
    data[1] = i>>16;
    data[2] = i>>8;
    data[3] = i;
    return f->write(f, &data[0], sizeof(data) );
}

int ICACHE_FLASH_ATTR deserialize_string(serializer_t *f, char *dest, unsigned *size, unsigned maxsize)
{
    uint16_t len = 0;
    int r;
    do {
        r = deserialize_uint16(f,&len);
        if (r<0)
            break;
        if (len<maxsize) {
            r = -1;
            break;
        }

        if (NULL!=size) {
            *size = len;
        }
        r = deserialize_bytearray(f, (unsigned char*)dest, len);
    } while (0);
    return r;
}

int ICACHE_FLASH_ATTR deserialize_bytearray(serializer_t *f, unsigned char *dest, unsigned size)
{
    int r = f->read(f, dest, size);
    return r;
}

int ICACHE_FLASH_ATTR deserialize_int8(serializer_t *f, int8_t *i)
{
    return deserialize_uint8(f, (uint8_t*)i);
}

int ICACHE_FLASH_ATTR deserialize_uint8(serializer_t *f, uint8_t *i)
{
    uint8_t data;
    int r;
    r = f->read(f, &data, sizeof(data));
    if (r==0) {
        *i = data;
    }
    return r;
}

int ICACHE_FLASH_ATTR deserialize_int16(serializer_t *f, int16_t *i)
{
    return deserialize_uint16(f,(uint16_t*)i);
}

int ICACHE_FLASH_ATTR deserialize_uint16(serializer_t *f, uint16_t *i)
{
    uint8_t data[2];
    uint16_t val;
    int r;
    r = f->read(f, &data[0], sizeof(data));
    if (r==0) {
        val = (uint16_t)data[0]<<8;
        val += (uint16_t)data[1];
        *i = val;
    }
    return r;
}

int ICACHE_FLASH_ATTR deserialize_int32(serializer_t *f, int32_t *i)
{
    return deserialize_uint32(f,(uint32_t*)i);
}

int ICACHE_FLASH_ATTR deserialize_uint32(serializer_t *f, uint32_t *i)
{
    uint8_t data[4];
    uint32_t val;
    int r;
    r = f->read(f, &data[0], sizeof(data));
    if (r==0) {
        val = (uint32_t)data[0]<<24;
        val += (uint32_t)data[1]<<16;
        val += (uint32_t)data[2]<<8;
        val += (uint32_t)data[3];
        *i = val;
    }
    return r;
}

