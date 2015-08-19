#include "schedule.h"
#include "widget.h"
#include <stdbool.h>
#include "alloc.h"
#include <string.h>
#include <stdlib.h>

typedef struct schedule_entry {
    schedule_type_t type;
    union {
        unsigned int intval;
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


void schedule_event()
{
    screen_t *s;
#ifdef __linux__
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
#ifdef __linux__
                printf("New schedule, type %d\n", current->type);
#endif
                switch (current->type) {
                case SCHEDULE_SELECT:
                    s = screen_find(current->val.string);
                    if (s) {
                        screen_select(s);
                    } else {
#ifdef __linux__
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

void schedule_reset()
{
    current = NULL;
    while (schedule_root) {
        schedule_entry_t *e = schedule_root;
        schedule_root = schedule_root->next;
        os_free(e);
    }
}

void schedule_stop()
{
    schedule_running = false;
    current = schedule_root;
}

void schedule_start()
{
    current = schedule_root;
    schedule_running = true;
}

int schedule_append(schedule_type_t type, void *arg)
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
