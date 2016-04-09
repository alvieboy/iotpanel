#ifndef __SERDES_H__
#define __SERDES_H__

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "protos.h"

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

int WUNUSED serialize_string(serializer_t *f, const char *str);
int WUNUSED serialize_bytearray(serializer_t *f, const unsigned char *ptr, unsigned size);
int WUNUSED serialize_int8(serializer_t *f, int8_t i);
int WUNUSED serialize_uint8(serializer_t *f, uint8_t i);
int WUNUSED serialize_int16(serializer_t *f, int16_t i);
int WUNUSED serialize_uint16(serializer_t *f, uint16_t i);
int WUNUSED serialize_int32(serializer_t *f, int32_t i);
int WUNUSED serialize_uint32(serializer_t *f, uint32_t i);

int WUNUSED deserialize_string(serializer_t *f, char *dest, unsigned *size, unsigned maxsize);
char *deserialize_string_alloc(serializer_t *f);
int WUNUSED deserialize_bytearray(serializer_t *f, unsigned char *dest, unsigned size);
int WUNUSED deserialize_int8(serializer_t *f, int8_t *i);
int WUNUSED deserialize_uint8(serializer_t *f, uint8_t *i);
int WUNUSED deserialize_int16(serializer_t *f, int16_t *i);
int WUNUSED deserialize_uint16(serializer_t *f, uint16_t *i);
int WUNUSED deserialize_int32(serializer_t *f, int32_t *i);
int WUNUSED deserialize_uint32(serializer_t *f, uint32_t *i);

#endif
