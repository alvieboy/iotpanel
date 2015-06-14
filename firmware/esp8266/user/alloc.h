#ifndef __ALLOC_H__
#define __ALLOC_H__

extern void *pvPortMalloc( size_t xWantedSize );
extern void vPortFree(void *);

#define os_malloc pvPortMalloc
#define os_free vPortFree

#endif
