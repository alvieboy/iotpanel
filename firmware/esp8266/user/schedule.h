#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

typedef enum {
    SCHEDULE_SELECT,
    SCHEDULE_WAIT
} schedule_type_t;

void schedule_stop();
void schedule_start();
void schedule_reset();
int schedule_append( schedule_type_t type, void *arg );

void schedule_event();

#endif
