#include <memory.h>
#include <string.h>

#include "engine.h"
#include "logging.h"

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

int main(int argc, char ** argv)
{
    int status = 0;
    engine_ctx_t ctx;

#ifdef _DEBUG
    log_init();
#endif

    LOG_DEBUG("enter\n");

    LOG_DEBUG("initializing engine\n");
    memset(&ctx, 0, sizeof(ctx));
    ctx.window_width = 640;
    ctx.window_height = 480;
    strncpy(ctx.window_title, "fps", sizeof(ctx.window_title));
    if ((status = engine_init(&ctx)) != status_success)
    {
        LOG_ERROR("engine_init failed (%d)\n", status);
        return 1;
    }

    LOG_DEBUG("registering key callback with engine\n");
    if ((status = engine_register_key_callback(NULL, -1, -1, -1, -1, key_callback)) != status_success)
    {
        LOG_ERROR("engine_register_key_callback failed (%d)\n", status);
        return 2;
    }

    if ((status = engine_run()) != status_success)
    {
        LOG_ERROR("engine_run failed (%d)\n", status);
        return 3;
    }

    LOG_DEBUG("exit\n");

    return 0;
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    LOG_DEBUG("window = %p, key = %d, scancode = %d, action = %d, mods = %02x\n",
            window,  key, scancode, action, mods);
}
