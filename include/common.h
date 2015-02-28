#ifndef __COMMON_H__
#define __COMMON_H__

#include <GLFW/glfw3.h>
#ifdef __APPLE__
#   include <OpenGL/glu.h>
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/glu.h>
#endif
#include <FTGL/ftgl.h>
#include <stdlib.h>

#endif

