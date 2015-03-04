#include "logging.h"

#include "common.h"

void * safe_alloc(unsigned int size)
{
    void * buf = NULL;
    
    if (size == 0)
    {
        LOG_ERROR("size is 0\n");
        return NULL;
    }

    if (!(buf = malloc(size)))
    {
        LOG_ERROR("failed to allocate memory\n");
        return NULL;
    }

    memset(buf, 0, size);

    return buf;
}

status_e safe_free(void * buf)
{

    if (!buf)
    {
        LOG_ERROR("buf is NULL!\n");
        return status_error;
    }

    free(buf);
    buf = NULL;


    return status_success;
}
