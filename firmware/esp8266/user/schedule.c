#include "schedule.h"
#include "widget.h"
#include <stdbool.h>
#include "alloc.h"
#include <string.h>
#include <stdlib.h>
#include "serdes.h"

typedef struct schedule_entry {
    schedule_type_t type;
    union {
        uint32_t intval;
        char string[NAMELEN+1];
    } val;
    struct schedule_entry *next;
} schedule_entry_t;

LOCAL schedule_entry_t *schedule_root = NULL;
LOCAL schedule_entry_t *current = NULL;

LOCAL bool schedule_running = false;

LOCAL int current_tick = 0;
LOCAL int current_delay = 0;
#define SCHEDULE_TICKS 10


void ICACHE_FLASH_ATTR schedule_event()
{
    screen_t *s;
    if (!schedule_running)
        return;
#ifdef DEBUG
    printf("Schedule: current tick %d, delay %d, cptr %p\n",
           current_tick, current_delay, current);
#endif
    current_tick++;
    if (current_tick==SCHEDULE_TICKS) {
        current_tick=0;
        if (current_delay) {
            current_delay--;
        } else {
            // Handle event.
            if (current) {
#ifdef DEBUG
                printf("New schedule, type %d\n", current->type);
#endif
                switch (current->type) {
                case SCHEDULE_SELECT:
                    s = screen_find(current->val.string);
                    if (s) {
                        screen_select(s);
                    } else {
#ifdef DEBUG
                        printf("Cannot find screen\n");
                        abort();
#endif
                    }
                    break;
                case SCHEDULE_WAIT:
                    current_delay = current->val.intval * 10;
                    break;
                }
                current = current->next;
                if (current==NULL) {
                    current = schedule_root;
                }
            }
        }
    }
}

void ICACHE_FLASH_ATTR schedule_reset()
{
    current = NULL;
    while (schedule_root) {
        schedule_entry_t *e = schedule_root;
        schedule_root = schedule_root->next;
        os_free(e);
    }
}

void ICACHE_FLASH_ATTR schedule_stop()
{
    schedule_running = false;
    current = schedule_root;
}

void ICACHE_FLASH_ATTR schedule_start()
{
    current = schedule_root;
    schedule_running = true;
}

int ICACHE_FLASH_ATTR schedule_append(schedule_type_t type, void *arg)
{
    char *end;
    schedule_entry_t *s = (schedule_entry_t*)os_malloc(sizeof(schedule_entry_t));
    s->next = NULL;
    s->type = type;
    switch (type) {
    case SCHEDULE_SELECT:
        strncpy(s->val.string, arg, sizeof(s->val.string));
        break;

    case SCHEDULE_WAIT:
        s->val.intval = (unsigned int)strtol(arg,&end,10);
        if (end && *end!='\0') {
            os_free(s);
            return -1;
        }
    }
    /* Append it */
    schedule_entry_t *tail = schedule_root;
    if (schedule_root == NULL) {
        schedule_root = current = s;
        return 0;
    }
    while (tail->next) {
        tail=tail->next;
    }
    tail->next = s;
    return 0;
}

int ICACHE_FLASH_ATTR schedule_serialize(serializer_t *ser)
{
    serialize_uint8(ser, schedule_running ? 1: 0);
    schedule_entry_t *h = schedule_root;
    while (h) {
        serialize_uint8( ser, h->type );
        switch (h->type) {
        case SCHEDULE_SELECT:
            serialize_string(ser, h->val.string);
            break;
        case SCHEDULE_WAIT:
            serialize_uint32(ser, h->val.intval);
            break;
        }
        h = h->next;
    }
    serialize_uint8(ser, 0); //  Last entry

    return 0;
}

int ICACHE_FLASH_ATTR schedule_deserialize(serializer_t *ser)
{
    uint8_t running;
    uint8_t type;
    unsigned ssize;
    int r = 0;

    schedule_reset();

    if (deserialize_uint8(ser, &running)<0)
        return -1;

    do {
        schedule_entry_t *last = NULL;

        if (deserialize_uint8( ser, &type )<0)
            return -1;

        if (type==0)
            break; // last one

        schedule_entry_t *s = (schedule_entry_t*)os_malloc(sizeof(schedule_entry_t));
        // schedule_root is NULL initially

        s->type = type;
        switch (type) {
        case SCHEDULE_SELECT:
            if (deserialize_string(ser, s->val.string, &ssize, sizeof(s->val.string))<0)
                r=-1;
            break;
        case SCHEDULE_WAIT:
            if (deserialize_uint32(ser, &s->val.intval)<0)
                r=-1;
            break;
        }
        if (r<0)
            break;

        if (NULL==last) {
            schedule_root = s;
        } else {
            last->next = s;
        }
        last = s;

    } while (1);

    if (r<0) {
        schedule_reset();
    } else {
        schedule_running  = running;
    }

    return r;
}
