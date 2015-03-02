#ifndef __RENDER_H__
#define __RENDER_H__

#include "common.h"

typedef struct
{
    GLfloat pos[4];
    GLfloat color[4];
    GLfloat scale[3];
    GLfloat rotation_angle;
    GLfloat rotation_vector[3];
} render_ctx_t;

void render_cube(const render_ctx_t * ctx);

#endif  // __RENDER_H__
