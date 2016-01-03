#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "serdes.h"

typedef enum {
    SCHEDULE_SELECT=1,
    SCHEDULE_WAIT
} schedule_type_t;

void schedule_stop();
void schedule_start();
void schedule_reset();
int schedule_append( schedule_type_t type, void *arg );

void schedule_event();
int schedule_serialize(serializer_t *);
int schedule_deserialize(serializer_t *);

#endif
