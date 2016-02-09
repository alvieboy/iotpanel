#ifndef __PROTOS_H__
#define __PROTOS_H__

#include "ets_sys.h"
#include "osapi.h"

int ets_printf(const char*fmt,...);
int os_printf(const char*fmt,...);
int os_sprintf(char *buf, const char*fmt,...);
int os_printf_plus(const char*fmt,...);
int ets_sprintf(char*,const char*fmt,...);
void *ets_memset(void *s, int c, size_t n);
char *os_strdup(const char *);
void ets_isr_unmask(int);
void ets_isr_mask(int);
void ets_delay_us(uint32);
void *ets_memcpy(void *,const void*,size_t);
void ets_isr_attach(int, void(*)(void), void*);

extern void pp_soft_wdt_stop();
extern void pp_soft_wdt_restart();
extern void slop_wdt_feed();

extern void ets_intr_lock();
extern void ets_intr_unlock();

extern void NmiTimSetFunc(void(*)(void));

int abs(int);

void system_restart();

unsigned system_get_free_heap_size();

#endif
