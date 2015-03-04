#include "array.h"
#include "logging.h"

#include "render.h"

static int __initialized = 0;
static array_t __objects;
static array_t __defs;
static void __render_shutdown(void);
static status_e __ctx_sanity_check(const render_ctx_t * ctx);
static status_e __def_sanity_check(const render_def_t * def);

// cube ///////////////////////////////////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

// vertex coords array for glDrawArrays() =====================================
// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
// array is 108 floats (36 * 3 = 108).
GLfloat __cube_vertices[] = { 
                        0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,      // v0-v1-v2 (front)
                       -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,      // v2-v3-v0
                        0.5f, 0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f,-0.5f,-0.5f,      // v0-v3-v4 (right)
                        0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,      // v4-v5-v0
                        0.5f, 0.5f, 0.5f,   0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,      // v0-v5-v6 (top)
                       -0.5f, 0.5f,-0.5f,  -0.5f, 0.5f, 0.5f,   0.5f, 0.5f, 0.5f,      // v6-v1-v0
                       -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,      // v1-v6-v7 (left)
                       -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,      // v7-v2-v1
                       -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,      // v7-v4-v3 (bottom)
                        0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f,-0.5f,-0.5f,      // v3-v2-v7
                        0.5f,-0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,      // v4-v7-v6 (back)
                       -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f,-0.5f,-0.5f,      // v6-v5-v4
};    

// normal array
GLfloat __cube_normals[]  = { 
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7
                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v6-v5-v4
}; 

status_e render_init(void)
{
    if (__initialized)
    {
        LOG_ERROR("renderer already initialized\n");
        return status_error;
    }

    LOG_DEBUG("initializing renderer...\n");
   
    atexit(__render_shutdown);
    
    if (array_init(&__objects) != status_success)
    {
        LOG_ERROR("failed to allocate memory for object array\n");
        return status_error;
    }

    if (array_init(&__defs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for definition array\n");
        return status_error;
    }

    __initialized = 1;

    LOG_DEBUG("initialization complete\n");

    return status_success;
}

void render_objects(void)
{
    glMatrixMode(GL_MODELVIEW); 
    glPushMatrix();
    glLoadIdentity();

    //ARRAY_FOREACH(&__objects, ctx)
    for (unsigned long idx = 0; idx < __objects.len; ++idx)
    {
        //render_object((render_ctx_t *)ctx); 
        render_object(array_get(&__objects, idx)); 
    }

    glPopMatrix();
}

void render_object(const render_ctx_t * ctx)
{
    GLfloat * normals = NULL, * vertices = NULL;
    GLsizei num_vertices = 0;

    if (__ctx_sanity_check(ctx) != status_success) return;

    switch (ctx->object_type)
    {
        case render_object_cube:
            normals = __cube_normals;
            vertices = __cube_vertices;
            num_vertices = 36;
            break;
        default:
            break;
    }

    for (unsigned long idx = 0; !normals && idx < __defs.len; ++idx)
    {
        render_def_t * def = array_get(&__defs, idx);
        if (def && def->id == ctx->def_id)
        {
            normals = def->normals;
            vertices = def->vertices;
            num_vertices = def->num_vertices;
            break;
        }
    }

    if (!normals)
    {
        LOG_ERROR("normals array is NULL!\n");
        return;
    }

    if (!vertices)
    {
        LOG_ERROR("vertices array is NULL!\n");
        return;
    }

    if (num_vertices == 0)
    {
        LOG_ERROR("num vertices is 0!\n");
        return;
    }

    // TODO more efficient to render all the objects at once, so
    // add func like render_add_object(ctx) where ctx has normals, vertices and num_verts
    // with another func allowing user to specify pre-defined objects render_add_object(ctx, type)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glNormalPointer(GL_FLOAT, 0, normals);
    glVertexPointer(3, GL_FLOAT, 0, vertices);

    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glColor4fv(ctx->color);
    glTranslatef(ctx->pos[0], ctx->pos[1], ctx->pos[2]);
    glScalef(ctx->scale[0], ctx->scale[1], ctx->scale[2]);
    glRotatef(ctx->rotation_angle, ctx->rotation_vector[0], ctx->rotation_vector[1], ctx->rotation_vector[2]);

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glPopAttrib();
    glPopMatrix();

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

status_e render_add_object(render_ctx_t * ctx)
{
    status_e status = __ctx_sanity_check(ctx);
    if (status != status_success) return status;

    return array_push(&__objects, ctx);
}

status_e render_remove_object(render_ctx_t * ctx)
{
    status_e status = __ctx_sanity_check(ctx);
    if (status != status_success) return status;

    // TODO not implemented

    return status;
}

status_e render_add_def(render_def_t * def)
{
    status_e status = __def_sanity_check(def);
    if (status != status_success) return status;

    return array_push(&__defs, def);
}

status_e render_remove_def(render_def_t * def)
{
    status_e status = __def_sanity_check(def);
    if (status != status_success) return status;

    // TODO

    return status;
}

static void __render_shutdown(void)
{
    if (!__initialized)
    {
        LOG_ERROR("renderer already initialized!\n");
        return;
    }
    
    LOG_DEBUG("shutting down...\n");

    array_destroy_deep(&__objects);
    array_destroy_deep(&__defs);

    __initialized = 0;

    LOG_DEBUG("shutdown complete\n");
}

static status_e __ctx_sanity_check(const render_ctx_t * ctx)
{
    if (!ctx)
    {
        LOG_ERROR("ctx is NULL!\n");
        return status_error;
    }

    return status_success;
}

static status_e __def_sanity_check(const render_def_t * def)
{
    if (!def)
    {
        LOG_ERROR("def is NULL!\n");
        return status_error;
    }

    return status_success;
}
