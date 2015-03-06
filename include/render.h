#ifndef __RENDER_H__
#define __RENDER_H__

#include "common.h"

typedef enum
{
    render_object_cube = 0,
    render_object_count
} render_object_e;

typedef struct
{
    GLfloat pos[4];
    GLfloat color[4];
    GLfloat scale[3];
    GLfloat rotation_angle;
    GLfloat rotation_vector[3];
    render_object_e object_type;    // pre-defined object type whose def to use
    unsigned long def_id;           // points to the associated render def
} render_ctx_t;

typedef struct
{
    unsigned long id; // TODO hash table mapping id -> def
    GLfloat * vertices; 
    GLfloat * normals;
    GLsizei num_vertices;
    GLenum vertex_mode;
} render_def_t;

status_e render_init(void);
void render_objects(void);
void render_object(const render_ctx_t * ctx);
status_e render_add_object(render_ctx_t * ctx);
status_e render_remove_object(render_ctx_t * ctx);
status_e render_add_def(render_def_t * def);
status_e render_remove_def(render_def_t * def);

#endif  // __RENDER_H__
