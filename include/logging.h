#ifndef __LOGGING_H__
#define __LOGGING_H__

#ifdef _DEBUG

void log_init(void);
void __log_msg(const char * format, ...);
char * __timestamp(void);

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_BLACK         "\033[30m"
#define COLOR_DARKRED       "\033[31m"
#define COLOR_DARKGREEN     "\033[32m"
#define COLOR_DARKYELLOW    "\033[33m"
#define COLOR_DARKBLUE      "\033[34m"
#define COLOR_DARKMAGENTA   "\033[35m"
#define COLOR_DARKCYAN      "\033[36m"
#define COLOR_GRAY          "\033[37m"
#define COLOR_DARKGRAY      "\033[90m"
#define COLOR_RED           "\033[91m"
#define COLOR_WHITE         "\033[97m"
#define COLOR_END           "\033[0m"

#define LOG_MSG(color1, str1, color2, str2, format, ...) \
    do { \
        char * timestamp = __timestamp(); \
        fprintf(stderr, "(%s%s:%d:%s%s%s%s %s%s%s): " format, COLOR_DARKGRAY, __FILE__, __LINE__, COLOR_END, color1, str1, COLOR_END, color2, str2, COLOR_END, ##__VA_ARGS__); \
        __log_msg("(%s%s:%s:%d:%s%s%s%s %s%s%s): " format, COLOR_DARKGRAY, timestamp ? timestamp : "", __FILE__, __LINE__, COLOR_END, color1, str1, COLOR_END, color2, str2, COLOR_END, ##__VA_ARGS__); \
        if (timestamp) free(timestamp); \
    } while (0)

#define LOG_DEBUG(format, ...) LOG_MSG(COLOR_DARKCYAN, __FUNCTION__, COLOR_GRAY, "DEBUG", format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_MSG(COLOR_DARKCYAN, __FUNCTION__, COLOR_RED, "ERROR", format, ##__VA_ARGS__)

#else

#define LOG_DEBUG(format, ...)
#define LOG_ERROR(format, ...)

#endif  // _DEBUG

#endif  // __LOGGING_H__
