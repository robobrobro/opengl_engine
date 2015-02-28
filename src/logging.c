#ifdef _DEBUG

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "logging.h"

static int __log_initialized = 0;
static FILE * __log_fp = NULL;
static pthread_mutex_t __lock;

void log_init(void);
static void __log_shutdown(void);

void log_init(void)
{
    if (__log_initialized) return;

    pthread_mutex_init(&__lock, NULL);

    if (!__log_fp)
    {
        __log_fp = fopen(".debug.log", "a");
        if (!__log_fp)
        {
            LOG_MSG(COLOR_DARKCYAN, __FUNCTION__, COLOR_RED, "ERROR", "failed to open the log file (%s)\n", strerror(errno));
        }
    }

    atexit(__log_shutdown);

    __log_initialized = 1;
}

static void __log_shutdown(void)
{
    if (!__log_initialized) return;

    if (__log_fp)
    {
        if (fclose(__log_fp) != 0)
        {
            LOG_MSG(COLOR_DARKCYAN, __FUNCTION__, COLOR_RED, "ERROR", "failed to close the log file (%s)\n", strerror(errno));
        }
        __log_fp = NULL;
    }

    pthread_mutex_destroy(&__lock);

    __log_initialized = 0;
}

void __log_msg(const char * format, ...)
{
    va_list args;

    if (!__log_initialized) return;

    pthread_mutex_lock(&__lock);

    va_start(args, format);
    vfprintf(__log_fp, format, args); 
    va_end(args);

    pthread_mutex_unlock(&__lock);
}

char * __timestamp(void)
{
    char * buf = NULL;
    struct tm * tminfo;
    time_t now;

    if (!(buf = malloc(sizeof(char) * 20)))
    {
        LOG_MSG(COLOR_DARKCYAN, __FUNCTION__, COLOR_RED, "ERROR", "failed to allocate memory for timestamp buf (%s)\n",
                strerror(errno));
        return NULL;
    }

    time(&now);
    tminfo = localtime(&now);
    strftime(buf, 20, "%Y.%m.%d %H:%M:%S", tminfo);
    return buf;
}

#endif  // _DEBUG
