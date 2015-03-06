#include <memory.h>
#include <string.h>
#include <time.h>

#include "array.h"
#include "engine.h"
#include "logging.h"
#include "render.h"

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);
static void mouse_pos_callback(GLFWwindow * window, double xpos, double ypos);
static void mouse_enter_callback(GLFWwindow * window, int entered);
static void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
static void mouse_scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
static void framebuffer_size_callback(GLFWwindow * window, int width, int height);
static void render_callback(double delta);
static void update_callback(double delta);
static int __add_objects_to_scene(void);
static array_t __cubes;

int main(int argc, char ** argv)
{
    int status = 0;
    engine_ctx_t ctx;

#ifdef _DEBUG
    log_init();
#endif

    memset(&ctx, 0, sizeof(ctx));
    ctx.window_width = 640;
    ctx.window_height = 480;
    strncpy(ctx.window_title, "fps", sizeof(ctx.window_title));
    ctx.mouse_disabled = 1;
    if ((status = engine_init(&ctx)) != status_success)
    {
        LOG_ERROR("engine_init failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering key callback with engine\n");
    if ((status = engine_register_key_callback(NULL, -1, -1, -1, -1, key_callback)) != status_success)
    {
        LOG_ERROR("engine_register_key_callback failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering mouse pos callback with engine\n");
    if ((status = engine_register_mouse_pos_callback(NULL, mouse_pos_callback)) != status_success)
    {
        LOG_ERROR("engine_register_mouse_pos_callback failed (%d)\n", status);
        return status;
    }
    
    LOG_DEBUG("registering mouse enter callback with engine\n");
    if ((status = engine_register_mouse_enter_callback(NULL, -1, mouse_enter_callback)) != status_success)
    {
        LOG_ERROR("engine_register_mouse_enter_callback failed (%d)\n", status);
        return status;
    }
    
    LOG_DEBUG("registering mouse button callback with engine\n");
    if ((status = engine_register_mouse_button_callback(NULL, -1, -1, -1, mouse_button_callback)) != status_success)
    {
        LOG_ERROR("engine_register_mouse_button_callback failed (%d)\n", status);
        return status;
    }
    
    LOG_DEBUG("registering mouse scroll callback with engine\n");
    if ((status = engine_register_mouse_scroll_callback(NULL, mouse_scroll_callback)) != status_success)
    {
        LOG_ERROR("engine_register_mouse_scroll_callback failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering framebuffer size callback with engine\n");
    if ((status = engine_register_framebuffer_size_callback(NULL, framebuffer_size_callback)) != status_success)
    {
        LOG_ERROR("engine_register_framebuffer_size_callback failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering render callback with engine\n");
    if ((status = engine_register_render_callback(render_callback)) != status_success)
    {
        LOG_ERROR("engine_register_render_callback failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering update callback with engine\n");
    if ((status = engine_register_update_callback(update_callback)) != status_success)
    {
        LOG_ERROR("engine_register_update_callback failed (%d)\n", status);
        return status;
    }

    if (!__add_objects_to_scene()) return 1;

    if ((status = engine_run()) != status_success)
    {
        LOG_ERROR("engine_run failed (%d)\n", status);
        return status;
    }

    return 0;
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    LOG_DEBUG("window = %p, key = %d, scancode = %d, action = %d, mods = %02x\n",
            window,  key, scancode, action, mods);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

static void mouse_pos_callback(GLFWwindow * window, double xpos, double ypos)
{
    LOG_DEBUG("window = %p, pos = (%.2f, %.2f)\n", window, xpos, ypos);
}

static void mouse_enter_callback(GLFWwindow * window, int entered)
{
    LOG_DEBUG("window = %p, entered = %d\n", window, entered);
}

static void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    LOG_DEBUG("window = %p, button = %d, action = %d, mods = %02x\n", window, button, action, mods);
}

static void mouse_scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
    LOG_DEBUG("window = %p, offset = (%.2f, %.2f)\n", window, xoffset, yoffset);
}

static void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
    LOG_DEBUG("window = %p, width = %d, height = %d\n", window, width, height);
}

static void render_callback(double delta)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45, 1.333, 0.01, 100); // deprecated but multi-platform and functional
}

static void update_callback(double delta)
{
    // rotate cubes
    for (unsigned long idx = 0; idx < __cubes.len; ++idx)
    {
        render_ctx_t * ctx = (render_ctx_t *) array_get(&__cubes, idx);
        ctx->rotation_angle += delta * 5.0f;
    }
}

static void __remove_objects_from_scene(void)
{
    array_destroy(&__cubes);
}

static int __add_objects_to_scene(void)
{
    srand(time(0));

    array_init(&__cubes);
    atexit(__remove_objects_from_scene);
    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            render_ctx_t * ctx = calloc(1, sizeof(render_ctx_t));
            ctx->pos[0] = -2.25f + 1.5f * x;
            ctx->pos[1] = -0.75f + 1.5f * y;
            ctx->pos[2] = -10.0f;
            ctx->color[0] = rand() / (GLfloat)RAND_MAX;
            ctx->color[1] = rand() / (GLfloat)RAND_MAX;
            ctx->color[2] = rand() / (GLfloat)RAND_MAX;
            ctx->color[3] = 1.0f;
            ctx->scale[0] = 1.0f;
            ctx->scale[1] = 1.0f;
            ctx->scale[2] = 1.0f;
            ctx->rotation_angle = 0.0f;
            ctx->rotation_vector[0] = 0.0f;
            ctx->rotation_vector[1] = 1.0f;
            ctx->rotation_vector[2] = 0.0f;
            ctx->object_type = render_object_cube;
            if (array_push(&__cubes, ctx) != status_success) return 0;
    
            if (render_add_object(ctx) != status_success) return 0;
        } 
    }

    return 1;
}
