#include "common.h"
#include "logging.h"

#include "engine.h"

static int __engine_initialized = 0;
static engine_ctx_t * __ctx;

static void __engine_shutdown(void);
static void __error_callback(int error, const char * description);
static void __key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

status_e engine_init(engine_ctx_t * ctx)
{
    LOG_DEBUG("enter\n");

    if (__engine_initialized)
    {
        LOG_ERROR("engine already initialized!\n");
        return status_error;
    }

    if (!ctx)
    {
        LOG_ERROR("ctx is null!\n");
        return status_error;
    }

    __ctx = ctx;

    atexit(__engine_shutdown);

    __engine_initialized = 1;

    LOG_DEBUG("exit\n");
    return status_success;
}

static void __engine_shutdown(void)
{
    LOG_DEBUG("enter\n");

    __ctx = NULL;

    __engine_initialized = 0;

    LOG_DEBUG("exit\n");
}

status_e engine_run(void)
{
    GLFWwindow * window = NULL;

    LOG_DEBUG("enter\n");

    if (!__ctx)
    {
        LOG_ERROR("__ctx is null!\n");
        return status_error;
    }

    glfwSetErrorCallback(__error_callback);

    if (!glfwInit())
    {
        LOG_ERROR("failed to initialized GLFW\n");
        return status_error;
    }

    window = glfwCreateWindow(__ctx->window_width, __ctx->window_height, __ctx->window_title, NULL, NULL);
    if (!window)
    {
        LOG_ERROR("failed to create window\n");

        glfwTerminate();

        return status_error;
    }

    glfwSetKeyCallback(window, __key_callback);
    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        // TODO render
        
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    LOG_DEBUG("exit\n");
    return status_success;
}

static void __error_callback(int error, const char * description)
{
    LOG_ERROR("%s\n", description);
}

static void __key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    LOG_DEBUG("window = %p, key = %d, scancode = %d, action = %d, mods = %02x\n",
            window, key, scancode, action, mods);
}
