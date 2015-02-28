#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "common.h"

typedef enum
{
    status_success = 0,
    status_error,
} status_e;

typedef struct
{
    int window_width;
    int window_height;
    char window_title[100];
} engine_ctx_t;

status_e engine_init(engine_ctx_t * ctx);
status_e engine_run(void);

typedef void (*engine_key_cb)(GLFWwindow * window, int key, int scancode, int action, int mods);

status_e engine_register_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods, engine_key_cb cb);

#endif
