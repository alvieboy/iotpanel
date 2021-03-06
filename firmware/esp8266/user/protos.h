#ifndef __PROTOS_H__
#define __PROTOS_H__

#include "ets_sys.h"
#include "osapi.h"

int ets_printf(const char*fmt,...);
int os_printf(const char*fmt,...);
int ets_sprintf(char*,const char*fmt,...);
void ets_isr_unmask(int);
void ets_delay_us(uint32);
void *ets_memcpy(void *,const void*,size_t);
#endif
