#include "logging.h"

#include "array.h"

static status_e __array_sanity_check(const array_t * a);
static status_e __array_resize(array_t * a, unsigned int size);
static status_e __array_destroy(array_t * a, int deep);

status_e array_init(array_t * a)
{
    if (!a)
    {
        LOG_ERROR("array is NULL!\n");
        return status_error;
    }

    if (a->len)
    {
        LOG_ERROR("array is not empty (len = %d)!\n", a->len);
        return status_error;
    }

    if (a->data)
    {
        LOG_ERROR("data is not NULL! (a->data = %p)\n", a->data);
        return status_error;
    }

    if (a->capacity == 0) a->capacity = 10;

    if (!(a->data = calloc(a->capacity, sizeof(void *))))
    {
        LOG_ERROR("failed to allocate memory for array\n");
        return status_error;
    }

    a->len = 0;
    
    return status_success;
}

status_e array_destroy(array_t * a)
{
    return __array_destroy(a, 0);
}

status_e array_destroy_deep(array_t * a)
{
    return __array_destroy(a, 1);
}

static status_e __array_destroy(array_t * a, int deep)
{
    unsigned int idx = 0;


    __array_sanity_check(a);

    if (!a || !a->data) return status_error;

    for (idx = 0; deep && idx < a->len; ++idx)
    {
        void * ptr = array_get(a, idx);
        if (ptr)
        {
            free(ptr);
            array_set(a, idx, NULL);
        }
    }

    free(a->data);
    a->data = NULL;
    a->len = 0;
    a->capacity = 0;

    return status_success;
}

status_e array_push(array_t * a, void * elem)
{
    status_e status = status_success;

    if ((status = __array_sanity_check(a)) != status_success)
    {
        return status;
    }

    if (a->len == a->capacity)
    {
        if ((status = __array_resize(a, a->capacity * 2)) != status_success)
        {
            return status;
        }
    }

    a->data[a->len++] = elem;

    return status;
}

void * array_pop(array_t * a)
{
    void * elem = NULL;

    if (__array_sanity_check(a) != status_success) return NULL;

    if (a->len == 0)
    {
        LOG_ERROR("array %p: length is 0!\n", a);
        return NULL;
    }

    elem = a->data[--a->len]; 

    if (a->len <= a->capacity / 2)
    {
        if (__array_resize(a, a->capacity / 2) != status_success)
        {
            return NULL;
        }
    }

    return elem;
}

void * array_get(const array_t * a, unsigned int idx)
{
    if (__array_sanity_check(a) != status_success) 
    {
        return NULL;
    }

    if (idx >= a->len)
    {
        LOG_ERROR("idx is out of bounds (idx = %d, len = %d)\n", idx, a->len);
        return NULL;
    }

    return a->data[idx];
}

status_e array_set(array_t * a, unsigned int idx, void * elem)
{
    status_e status = status_success;


    if ((status = __array_sanity_check(a)) != status_success)
    {
        return status;
    }

    if (idx >= a->len)
    {
        LOG_ERROR("idx is out of bounds (idx = %d, len = %d)\n", idx, a->len);
        return status_error;
    }

    a->data[idx] = elem;

    return status;
}

static status_e __array_sanity_check(const array_t * a)
{
    if (!a)
    {
        LOG_ERROR("array is NULL!\n");
        return status_error;
    }

    if (!a->data)
    {
        LOG_ERROR("array %p: data is NULL!\n", a);
        return status_error;
    }

    if (a->len > a->capacity)
    {
        LOG_ERROR("array %p: overflow! len = %d, capacity = %d\n", a, a->len, a->capacity);
        return status_error;
    }

    return status_success;
}

static status_e __array_resize(array_t * a, unsigned int size)
{
    status_e status = status_success;
    unsigned int idx = 0;


    if ((status = __array_sanity_check(a)) != status_success)
    {
        return status;
    }

    if (size == a->capacity)
    {
        LOG_ERROR("array %p is already the specified size (%d)\n", a, size);
        return status_error;
    }
    else if (size < a->capacity)
    {
        for (idx = size; idx < a->capacity; ++idx)
        {
            if (a->data[idx])
            {
                LOG_ERROR("array %p has non-NULL data at idx>=%d. resizing to a smaller capacity will cause a memory leak!\n", 
                        a, size);
                return status_error;
            }
        }
    }
    
    if (!(a->data = realloc(a->data, size)))
    {
        LOG_ERROR("array %p failed to resize to size %d\n", a, size);
        return status_error;
    }

    if (size > a->capacity)
    {
        memset(a->data + a->capacity, 0, size - a->capacity);
    }

    a->capacity = size;
    if (a->len > a->capacity) a->len = a->capacity;

    return status;
}
