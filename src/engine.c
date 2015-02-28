#include "logging.h"

#include "engine.h"

static int __engine_initialized = 0;
static engine_ctx_t * __ctx;
typedef struct
{
    GLFWwindow * window;
    int key;
    int scancode;
    int action;
    int mods;
    engine_key_cb callback;
} __key_cb_ctx_t;
static __key_cb_ctx_t * __key_cbs = NULL;
static unsigned int __key_cbs_len = 0;
static unsigned int __key_cbs_capacity = 0;

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

    if (!(__key_cbs = malloc(sizeof(__key_cb_ctx_t) * 10)))
    {
        LOG_ERROR("failed to allocate memory for key callback array\n");
        return status_error;
    }

    __key_cbs_len = 0;
    __key_cbs_capacity = 0;

    atexit(__engine_shutdown);

    __engine_initialized = 1;

    LOG_DEBUG("exit\n");
    return status_success;
}

static void __engine_shutdown(void)
{
    LOG_DEBUG("enter\n");

    if (__key_cbs)
    {
        free(__key_cbs);
        __key_cbs = NULL;
        __key_cbs_len = 0;
        __key_cbs_capacity = 0;
    }

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
    unsigned int key_cb_idx = 0;

    LOG_DEBUG("window = %p, key = %d, scancode = %d, action = %d, mods = %02x\n",
            window, key, scancode, action, mods);

    // TODO hash table
    for (key_cb_idx = 0; key_cb_idx < __key_cbs_len; ++key_cb_idx)
    {
        __key_cb_ctx_t * ctx = &__key_cbs[key_cb_idx];
        LOG_DEBUG("ctx->window = %p, ctx->key = %d, ctx->scancode = %d, ctx->action = %d, ctx->mods = %02x, ctx->callback = %p\n",
                ctx->window, ctx->key, ctx->scancode, ctx->action, ctx->mods, ctx->callback);
        if (ctx->window != NULL && ctx->window != window) continue;
        if (ctx->key != -1 && ctx->key != key) continue;
        if (ctx->scancode != -1 && ctx->scancode != scancode) continue;
        if (ctx->action != -1 && ctx->action != action) continue;
        if (ctx->mods != -1 && ctx->mods != mods) continue;

        if (ctx->callback)
        {
            ctx->callback(window, key, scancode, action, mods);
        }
    } 
}

status_e engine_register_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods, engine_key_cb cb)
{
    __key_cb_ctx_t * ctx = NULL;
    
    LOG_DEBUG("enter\n");

    if (__key_cbs_len == __key_cbs_capacity)
    {
        if (!(__key_cbs = realloc(__key_cbs, sizeof(__key_cb_ctx_t) * __key_cbs_capacity * 2)))
        {
            LOG_ERROR("failed to reallocate memory for the key callback array\n");
            return status_error;
        }

        __key_cbs_capacity *= 2;
    }

    ctx = &__key_cbs[__key_cbs_len++];
    ctx->window = window;
    ctx->key = key;
    ctx->scancode = scancode;
    ctx->action = action;
    ctx->mods = mods;
    ctx->callback = cb;

    LOG_DEBUG("ctx #%d registered:\n", __key_cbs_len);
    LOG_DEBUG("ctx->window = %p, ctx->key = %d, ctx->scancode = %d, ctx->action = %d, ctx->mods = %d, ctx->callback = %p\n",
            ctx->window, ctx->key, ctx->scancode, ctx->action, ctx->mods, ctx->callback);

    LOG_DEBUG("exit\n");

    return status_success;
}
