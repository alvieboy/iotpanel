//////////////////////////////////////////////////
// Mutex support for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <os_type.h>

typedef int32_t mutex_t;
#define DEFAULT_MUTEX_INITIALIZER 1

void CreateMutex(mutex_t *mutex);
bool GetMutex(mutex_t *mutex);
void ReleaseMutex(mutex_t *mutex);
void LockMutex(mutex_t *mutex);

#endif
