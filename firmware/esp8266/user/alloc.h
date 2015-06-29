#ifndef __ALLOC_H__
#define __ALLOC_H__

extern void *pvPortMalloc( size_t xWantedSize );
extern void *pvPortCalloc( size_t xWantedSize, int iNum );
extern void vPortFree(void *);

#define os_malloc pvPortMalloc
#define os_free vPortFree
#define os_calloc pvPortCalloc

#endif
