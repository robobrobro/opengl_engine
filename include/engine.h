#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "common.h"

typedef struct
{
    int window_width;
    int window_height;
    char window_title[100];
    int mouse_disabled;
} engine_ctx_t;

status_e engine_init(engine_ctx_t * ctx);
status_e engine_run(void);

typedef void (*engine_key_cb)(GLFWwindow * window, int key, int scancode, int action, int mods);
typedef void (*engine_mouse_pos_cb)(GLFWwindow * window, double xpos, double ypos);
typedef void (*engine_mouse_enter_cb)(GLFWwindow * window, int entered);
typedef void (*engine_mouse_button_cb)(GLFWwindow * window, int button, int action, int mods);
typedef void (*engine_mouse_scroll_cb)(GLFWwindow * window, double xoffset, double yoffset);
typedef void (*engine_framebuffer_size_cb)(GLFWwindow * window, int width, int height);
typedef void (*engine_render_cb)();
typedef void (*engine_update_cb)(double delta);
typedef void (*engine_prerun_cb)(void);

status_e engine_register_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods, engine_key_cb cb);
status_e engine_register_mouse_pos_callback(GLFWwindow * window, engine_mouse_pos_cb cb);
status_e engine_register_mouse_enter_callback(GLFWwindow * window, int entered, engine_mouse_enter_cb cb);
status_e engine_register_mouse_button_callback(GLFWwindow * window, int button, int action, int mods, engine_mouse_button_cb cb);
status_e engine_register_mouse_scroll_callback(GLFWwindow * window, engine_mouse_scroll_cb cb);
status_e engine_register_framebuffer_size_callback(GLFWwindow * window, engine_framebuffer_size_cb cb);
status_e engine_register_render_callback(engine_render_cb cb);
status_e engine_register_postrender_callback(engine_render_cb cb);
status_e engine_register_update_callback(engine_update_cb cb);
status_e engine_register_prerun_callback(engine_prerun_cb cb);

#endif
