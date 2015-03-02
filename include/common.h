#ifndef __COMMON_H__
#define __COMMON_H__

#include <GLFW/glfw3.h>
#ifdef __APPLE__
#   define __gl_h_
#   define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#   include <OpenGL/gl3.h>
#   include <OpenGL/glu.h>
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/glu.h>
#endif
#include <FTGL/ftgl.h>
#include <stdlib.h>

typedef enum
{
    status_success = 0,
    status_error,
} status_e;

void * safe_alloc(unsigned int size);
status_e safe_free(void * buf);

#endif

