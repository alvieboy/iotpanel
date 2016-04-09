#ifndef __SERDES_H__
#define __SERDES_H__

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

typedef struct serializer_t
{
    int (*initialize)(struct serializer_t *me);
    int (*write)(struct serializer_t *me, const void *data, unsigned size);
    int (*read)(struct serializer_t *me, void *data, unsigned size);
    int (*rewind)(struct serializer_t *me);
    int (*truncate)(struct serializer_t *me);
    int (*finalise)(struct serializer_t *me);
    int (*release)(struct serializer_t *me);
    void *pvt;
} serializer_t;

int serialize_string(serializer_t *f, const char *str);
int serialize_bytearray(serializer_t *f, const unsigned char *ptr, unsigned size);
int serialize_int8(serializer_t *f, int8_t i);
int serialize_uint8(serializer_t *f, uint8_t i);
int serialize_int16(serializer_t *f, int16_t i);
int serialize_uint16(serializer_t *f, uint16_t i);
int serialize_int32(serializer_t *f, int32_t i);
int serialize_uint32(serializer_t *f, uint32_t i);

int deserialize_string(serializer_t *f, char *dest, unsigned *size, unsigned maxsize);
char *deserialize_string_alloc(serializer_t *f);
int deserialize_bytearray(serializer_t *f, unsigned char *dest, unsigned size);
int deserialize_int8(serializer_t *f, int8_t *i);
int deserialize_uint8(serializer_t *f, uint8_t *i);
int deserialize_int16(serializer_t *f, int16_t *i);
int deserialize_uint16(serializer_t *f, uint16_t *i);
int deserialize_int32(serializer_t *f, int32_t *i);
int deserialize_uint32(serializer_t *f, uint32_t *i);

#endif
