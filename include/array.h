#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "common.h"

typedef struct
{
    unsigned int len;
    unsigned int capacity;
    void ** data;
} array_t;

status_e array_init(array_t * a);
status_e array_destroy(array_t * a);
status_e array_destroy_deep(array_t * a);
void * array_get(const array_t * a, unsigned int idx);
status_e array_set(array_t * a, unsigned int idx, void * elem);
status_e array_push(array_t * a, void * elem);
void * array_pop(array_t * a);

#endif  // __ARRAY_H__
