#include "array.h"
#include "logging.h"
#include "render.h"

#include "engine.h"

static int __engine_initialized = 0;
static int __glfw_initialized = 0;
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
static array_t __key_cbs;
typedef struct
{
    GLFWwindow * window;
    engine_mouse_pos_cb callback;
} __mouse_pos_cb_ctx_t;
static array_t __mouse_pos_cbs;
typedef struct
{
    GLFWwindow * window;
    int entered;
    engine_mouse_enter_cb callback;
} __mouse_enter_cb_ctx_t;
static array_t __mouse_enter_cbs;
typedef struct
{
    GLFWwindow * window;
    int button;
    int action;
    int mods;
    engine_mouse_button_cb callback;
} __mouse_button_cb_ctx_t;
static array_t __mouse_button_cbs;
typedef struct
{
    GLFWwindow * window;
    engine_mouse_scroll_cb callback;
} __mouse_scroll_cb_ctx_t;
static array_t __mouse_scroll_cbs;
typedef struct
{
    GLFWwindow * window;
    engine_framebuffer_size_cb callback;
} __framebuffer_size_cb_ctx_t;
static array_t __framebuffer_size_cbs;
static engine_render_cb __render_cb = NULL;
static engine_update_cb __update_cb = NULL;
static double __last_frame_update = 0.0;

static void __engine_shutdown(void);
static void __error_callback(int error, const char * description);
static void __key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);
static void __mouse_pos_callback(GLFWwindow * window, double xpos, double ypos);
static void __mouse_enter_callback(GLFWwindow * window, int entered);
static void __mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
static void __mouse_scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
static void __framebuffer_size_callback(GLFWwindow * window, int width, int height);

status_e engine_init(engine_ctx_t * ctx)
{
    int glfw_major, glfw_minor, glfw_rev;
    status_e status = status_success;

    if (__engine_initialized)
    {
        LOG_ERROR("engine already initialized!\n");
        return status_error;
    }
    
    LOG_DEBUG("initializing engine...\n");

    atexit(__engine_shutdown);

    if (!ctx)
    {
        LOG_ERROR("ctx is null!\n");
        return status_error;
    }

    __ctx = ctx;

    if (array_init(&__key_cbs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for key callback array\n");
        return status_error;
    }
    
    if (array_init(&__mouse_pos_cbs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for mouse pos callback array\n");
        return status_error;
    }
    
    if (array_init(&__mouse_enter_cbs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for mouse enter callback array\n");
        return status_error;
    }
    
    if (array_init(&__mouse_button_cbs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for mouse button callback array\n");
        return status_error;
    }
    
    if (array_init(&__mouse_scroll_cbs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for mouse scroll callback array\n");
        return status_error;
    }
    
    if (array_init(&__framebuffer_size_cbs) != status_success)
    {
        LOG_ERROR("failed to allocate memory for framebuffer size callback array\n");
        return status_error;
    }

    __render_cb = NULL;
    __update_cb = NULL;
    __last_frame_update = 0.0;
    
    LOG_DEBUG("initialized callback arrays\n");

    glfwSetErrorCallback(__error_callback);

    __glfw_initialized = 0;
        
    if (!glfwInit())
    {
        LOG_ERROR("failed to initialized GLFW\n");
        return status_error;
    }

    __glfw_initialized = 1;
    LOG_DEBUG("compiled against GLFW Version %s\n", glfwGetVersionString());
    glfwGetVersion(&glfw_major, &glfw_minor, &glfw_rev);
    LOG_DEBUG("running with GLFW v%d.%d.%d\n", glfw_major, glfw_minor, glfw_rev);

    if ((status = render_init()) != status_success)
    {
        LOG_ERROR("failed to initialized renderer!\n");
        return status;
    }

    __engine_initialized = 1;

    LOG_DEBUG("engine initialization complete\n");

    return status;
}

static void __engine_shutdown(void)
{
    unsigned int idx = 0;

    if (!__engine_initialized)
    {
        LOG_ERROR("engine not initialized\n");
        return;
    }

    LOG_DEBUG("shutting down...\n");

    if (__glfw_initialized)
    {
        glfwTerminate();
        __glfw_initialized = 0;
    }
    LOG_DEBUG("GLFW terminated\n");

    array_destroy_deep(&__key_cbs);
    array_destroy_deep(&__mouse_pos_cbs);
    array_destroy_deep(&__mouse_enter_cbs);
    array_destroy_deep(&__mouse_button_cbs);
    array_destroy_deep(&__mouse_scroll_cbs);
    array_destroy_deep(&__framebuffer_size_cbs);
    __render_cb = NULL;
    __update_cb = NULL;
    __last_frame_update = 0.0;
    LOG_DEBUG("callback arrays destroyed\n");

    __ctx = NULL;

    __engine_initialized = 0;

    LOG_DEBUG("shutdown complete\n");
}

status_e engine_run(void)
{
    GLint gl_major = 0, gl_minor = 0, gl_num_extensions = 0, gl_num_shading_lang_vers = 0;
    GLuint idx = 0;
    GLFWwindow * window = NULL;

    if (!__ctx)
    {
        LOG_ERROR("__ctx is null!\n");
        return status_error;
    }

    window = glfwCreateWindow(__ctx->window_width, __ctx->window_height, __ctx->window_title, NULL, NULL);
    if (!window)
    {
        LOG_ERROR("failed to create window\n");
        return status_error;
    }

    glfwSetKeyCallback(window, __key_callback);
    glfwSetCursorPosCallback(window, __mouse_pos_callback);
    if (__ctx->mouse_disabled)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    glfwSetMouseButtonCallback(window, __mouse_button_callback);
    glfwSetScrollCallback(window, __mouse_scroll_callback);
    
    LOG_DEBUG("window created and configured\n");
    LOG_DEBUG("starting main processing loop...\n");
        
    glfwMakeContextCurrent(window);
   
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    LOG_DEBUG("running with OpenGL v%d.%d (%s), vendor: %s, renderer: %s\n", 
            gl_major, gl_minor, glGetString(GL_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    glGetIntegerv(GL_NUM_EXTENSIONS, &gl_num_extensions);
    if (gl_num_extensions > 0)
    {
        LOG_DEBUG("OpenGL extensions enabled (%d):\n", gl_num_extensions);
        for (idx = 0; idx < gl_num_extensions; ++idx)
        {
            LOG_DEBUG(" -- %s\n", glGetStringi(GL_EXTENSIONS, idx));
        }
    }
    LOG_DEBUG("OpenGL Shading Language (primary) version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
#ifdef GL_NUM_SHADING_LANGUAGE_VERSIONS
    glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &gl_num_shading_lang_vers);
    if (gl_num_shading_lang_vers > 0)
    {
        LOG_DEBUG("OpenGL shading language versions:\n");
        for (idx = 0; idx < gl_num_shading_lang_vers; ++idx)
        {
            LOG_DEBUG(" -- %s\n", glGetStringi(GL_SHADING_LANGUAGE_VERSION, idx));
        }
    }
#endif

    while (!glfwWindowShouldClose(window))
    {
	double now = glfwGetTime();
	double delta = now - __last_frame_update;
	__last_frame_update = now;

	if (__update_cb) __update_cb(delta);

        if (__render_cb) __render_cb(delta);
        render_objects();
        
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    LOG_DEBUG("window destroyed\n");

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
    for (key_cb_idx = 0; key_cb_idx < __key_cbs.len; ++key_cb_idx)
    {
        __key_cb_ctx_t * ctx = array_get(&__key_cbs, key_cb_idx);
        if (!ctx)
        {
            LOG_ERROR("key callback ctx #%d is NULL!\n", key_cb_idx);
            continue;
        }

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

static void __mouse_pos_callback(GLFWwindow * window, double xpos, double ypos)
{
    unsigned int mouse_pos_cb_idx = 0;

    for (mouse_pos_cb_idx = 0; mouse_pos_cb_idx < __mouse_pos_cbs.len; ++mouse_pos_cb_idx)
    {
        __mouse_pos_cb_ctx_t * ctx = array_get(&__mouse_pos_cbs, mouse_pos_cb_idx);
        if (!ctx)
        {
            LOG_ERROR("mouse pos callback ctx is NULL!\n");
            continue;
        }

        if (ctx->window != NULL && ctx->window != window) continue;

        if (ctx->callback)
        {
            ctx->callback(window, xpos, ypos);
        }
    }
}

static void __mouse_enter_callback(GLFWwindow * window, int entered)
{
    unsigned int idx = 0;

    for (idx = 0; idx < __mouse_enter_cbs.len; ++idx)
    {
        __mouse_enter_cb_ctx_t * ctx = array_get(&__mouse_enter_cbs, idx);
        if (!ctx)
        {
            LOG_ERROR("mouse enter callback ctx is NULL!\n");
            continue;
        }

        if (ctx->window != NULL && ctx->window != window) continue;
        if (ctx->entered != -1 && ctx->entered != entered) continue;

        if (ctx->callback)
        {
            ctx->callback(window, entered);
        }
    }
}

static void __mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    unsigned int idx = 0;

    for (idx = 0; idx < __mouse_button_cbs.len; ++idx)
    {
        __mouse_button_cb_ctx_t * ctx = array_get(&__mouse_button_cbs, idx);
        if (!ctx)
        {
            LOG_ERROR("mouse button callback ctx is NULL!\n");
            continue;
        }

        if (ctx->window != NULL && ctx->window != window) continue;
        if (ctx->button != -1 && ctx->button != button) continue;
        if (ctx->action != -1 && ctx->action != action) continue;
        if (ctx->mods != -1 && ctx->mods != mods) continue;

        if (ctx->callback)
        {
            ctx->callback(window, button, action, mods);
        }
    }
}

static void __mouse_scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
    unsigned int idx = 0;

    for (idx = 0; idx < __mouse_scroll_cbs.len; ++idx)
    {
        __mouse_scroll_cb_ctx_t * ctx = array_get(&__mouse_scroll_cbs, idx);
        if (!ctx)
        {
            LOG_ERROR("mouse scroll callback ctx is NULL!\n");
            continue;
        }

        if (ctx->window != NULL && ctx->window != window) continue;

        if (ctx->callback)
        {
            ctx->callback(window, xoffset, yoffset);
        }
    }
}

static void __framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
    unsigned int idx = 0;

    LOG_DEBUG("window = %p, width = %d, height = %d\n", window, width, height);

    for (idx = 0; idx < __framebuffer_size_cbs.len; ++idx)
    {
        __framebuffer_size_cb_ctx_t * ctx = array_get(&__framebuffer_size_cbs, idx);
        if (!ctx)
        {
            LOG_ERROR("framebuffer size callback ctx is NULL!\n");
            continue;
        }

        if (ctx->window != NULL && ctx->window != window) continue;

        if (ctx->callback)
        {
            ctx->callback(window, width, height);
        }
    }
}

status_e engine_register_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods, engine_key_cb cb)
{
    status_e status = status_success;
    __key_cb_ctx_t * ctx = NULL;
    

    if (!(ctx = safe_alloc(sizeof(__key_cb_ctx_t))))
    {
        LOG_ERROR("failed to allocate memory for the key callback ctx\n");
        return status_error;
    }

    ctx->window = window;
    ctx->key = key;
    ctx->scancode = scancode;
    ctx->action = action;
    ctx->mods = mods;
    ctx->callback = cb;

    if ((status = array_push(&__key_cbs, ctx)) != status_success)
    {
        LOG_ERROR("failed to push key callback ctx to array\n");
        free(ctx);
        return status;
    }

    LOG_DEBUG("ctx #%d registered:\n", __key_cbs.len);
    LOG_DEBUG("ctx->window = %p, ctx->key = %d, ctx->scancode = %d, ctx->action = %d, ctx->mods = %d, ctx->callback = %p\n",
            ctx->window, ctx->key, ctx->scancode, ctx->action, ctx->mods, ctx->callback);


    return status;
}

status_e engine_register_mouse_pos_callback(GLFWwindow * window, engine_mouse_pos_cb cb)
{
    status_e status = status_success;
    __mouse_pos_cb_ctx_t * ctx = NULL;


    if (!(ctx = safe_alloc(sizeof(__mouse_pos_cb_ctx_t))))
    {
        LOG_ERROR("failed to allocate memory for mouse pos callback ctx\n");
        return status_error;
    }

    ctx->window = window;
    ctx->callback = cb;

    if ((status = array_push(&__mouse_pos_cbs, ctx)) != status_success)
    {
        LOG_ERROR("failed to push mouse pos callback ctx onto array\n");
        free(ctx);
        return status_error;
    }

    LOG_DEBUG("ctx #%d registered:\n", __mouse_pos_cbs.len);
    LOG_DEBUG("ctx->window = %p, ctx->callback = %p\n", ctx->window, ctx->callback);


    return status;
}

status_e engine_register_mouse_enter_callback(GLFWwindow * window, int entered, engine_mouse_enter_cb cb)
{
    status_e status = status_success;
    __mouse_enter_cb_ctx_t * ctx = NULL;


    if (!(ctx = safe_alloc(sizeof(__mouse_enter_cb_ctx_t))))
    {
        LOG_ERROR("failed to allocate memory for mouse enter callback ctx\n");
        return status_error;
    }

    ctx->window = window;
    ctx->entered = entered;
    ctx->callback = cb;

    if ((status = array_push(&__mouse_enter_cbs, ctx)) != status_success)
    {
        LOG_ERROR("failed to push mouse enter callback ctx onto array\n");
        free(ctx);
        return status_error;
    }

    LOG_DEBUG("ctx #%d registered:\n", __mouse_enter_cbs.len);
    LOG_DEBUG("ctx->window = %p, ctx->entered = %d, ctx->callback = %p\n", ctx->window, ctx->entered, ctx->callback);


    return status;
}

status_e engine_register_mouse_button_callback(GLFWwindow * window, int button, int action, int mods, engine_mouse_button_cb cb)
{
    status_e status = status_success;
    __mouse_button_cb_ctx_t * ctx = NULL;

    if (!(ctx = safe_alloc(sizeof(__mouse_button_cb_ctx_t))))
    {
        LOG_ERROR("failed to allocate memory for mouse button callback ctx\n");
        return status_error;
    }

    ctx->window = window;
    ctx->button = button;
    ctx->action = action;
    ctx->mods = mods;
    ctx->callback = cb;

    if ((status = array_push(&__mouse_button_cbs, ctx)) != status_success)
    {
        LOG_ERROR("failed to push mouse button callback ctx onto array\n");
        free(ctx);
        return status_error;
    }

    LOG_DEBUG("ctx #%d registered:\n", __mouse_button_cbs.len);
    LOG_DEBUG("ctx->window = %p, ctx->button = %d, ctx->action = %d, ctx->mods = %d, ctx->callback = %p\n", 
            ctx->window, ctx->button, ctx->action, ctx->mods, ctx->callback);

    return status;
}

status_e engine_register_mouse_scroll_callback(GLFWwindow * window, engine_mouse_scroll_cb cb)
{
    status_e status = status_success;
    __mouse_scroll_cb_ctx_t * ctx = NULL;

    if (!(ctx = safe_alloc(sizeof(__mouse_scroll_cb_ctx_t))))
    {
        LOG_ERROR("failed to allocate memory for mouse scroll callback ctx\n");
        return status_error;
    }

    ctx->window = window;
    ctx->callback = cb;

    if ((status = array_push(&__mouse_scroll_cbs, ctx)) != status_success)
    {
        LOG_ERROR("failed to push mouse scroll callback ctx onto array\n");
        free(ctx);
        return status_error;
    }

    LOG_DEBUG("ctx #%d registered:\n", __mouse_scroll_cbs.len);
    LOG_DEBUG("ctx->window = %p, ctx->callback = %p\n", ctx->window, ctx->callback);

    return status;
}

status_e engine_register_framebuffer_size_callback(GLFWwindow * window, engine_framebuffer_size_cb cb)
{
    status_e status = status_success;
    __framebuffer_size_cb_ctx_t * ctx = NULL;

    if (!(ctx = safe_alloc(sizeof(__framebuffer_size_cb_ctx_t))))
    {
        LOG_ERROR("failed to allocate memory for framebuffer size callback ctx\n");
        return status_error;
    }

    ctx->window = window;
    ctx->callback = cb;

    if ((status = array_push(&__framebuffer_size_cbs, ctx)) != status_success)
    {
        LOG_ERROR("failed to push framebuffer size callback ctx onto array\n");
        free(ctx);
        return status_error;
    }

    LOG_DEBUG("ctx #%d registered:\n", __framebuffer_size_cbs.len);
    LOG_DEBUG("ctx->window = %p, ctx->callback = %p\n", ctx->window, ctx->callback);

    return status;
}

status_e engine_register_render_callback(engine_render_cb cb)
{
    if (!cb)
    {
        LOG_ERROR("callback is NULL!\n");
        return status_error;
    }

    __render_cb = cb;

    return status_success;
}

status_e engine_register_update_callback(engine_update_cb cb)
{
    if (!cb)
    {
        LOG_ERROR("callback is NULL!\n");
        return status_error;
    }

    __update_cb = cb;

    return status_success;
}
