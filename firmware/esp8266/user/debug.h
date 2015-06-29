#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __linux__
#define DEBUG(x...) printf(x)
#else
#define DEBUG(x...) /* os_printf(x)*/
#endif

#endif
