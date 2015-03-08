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
static void render_callback();
static void postrender_callback();
static void update_callback(double delta);
static void setup_scene(void);
static int setup_lighting(void);
static int add_objects_to_scene(void);
static void remove_objects_from_scene(void);
static int __camera_movement_direction[3] = { 0 };
static GLdouble __camera_movement_inc[3] = { 1.0, 0.0, 0.5 };
static GLdouble __camera_pos[3] = { 0.0 };
static GLdouble __camera_rotation[2] = { 0.0 };
static int __camera_rotation_direction[2] = { 0 };
static GLdouble __camera_rotation_inc[2] = { 90.0, 60.0 };
static double __mouse_pos[2] = { 0.0 };
static double __mouse_pos_last_frame[2] = { 0.0 };
static array_t __cubes;
engine_ctx_t __engine_ctx;

int main(int argc, char ** argv)
{
    int status = 0;


#ifdef _DEBUG
    log_init();
#endif

    memset(&__engine_ctx, 0, sizeof(__engine_ctx));
    __engine_ctx.window_width = 1280;
    __engine_ctx.window_height = 720;
    strncpy(__engine_ctx.window_title, "cubeworld", sizeof(__engine_ctx.window_title));
    __engine_ctx.mouse_disabled = 1;
    if ((status = engine_init(&__engine_ctx)) != status_success)
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

    LOG_DEBUG("registering postrender callback with engine\n");
    if ((status = engine_register_postrender_callback(render_callback)) != status_success)
    {
        LOG_ERROR("engine_register_postrender_callback failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering update callback with engine\n");
    if ((status = engine_register_update_callback(update_callback)) != status_success)
    {
        LOG_ERROR("engine_register_update_callback failed (%d)\n", status);
        return status;
    }

    LOG_DEBUG("registering prerun callback with engine\n");
    if ((status = engine_register_prerun_callback(setup_scene)) != status_success)
    {
        LOG_ERROR("engine_register_prerun_callback failed (%d)\n", status);
        return status;
    }

    if ((status = engine_run()) != status_success)
    {
        LOG_ERROR("engine_run failed (%d)\n", status);
        return status;
    }

    return 0;
}

static void __update_camera_movement_direction(int axis, int direction, int action)
{
    __camera_movement_direction[axis] = (action == GLFW_PRESS || action == GLFW_REPEAT ? direction : 0);
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    LOG_DEBUG("window = %p, key = %d, scancode = %d, action = %d, mods = %02x\n",
            window,  key, scancode, action, mods);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
    {
	__update_camera_movement_direction(0, 1, action);
    }
    else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
    {
	__update_camera_movement_direction(0, -1, action);
    }

    if (key == GLFW_KEY_UP || key == GLFW_KEY_W)
    {
	__update_camera_movement_direction(2, 1, action);
    }
    else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
    {
	__update_camera_movement_direction(2, -1, action);
    }
}

static void mouse_pos_callback(GLFWwindow * window, double xpos, double ypos)
{
    LOG_DEBUG("window = %p, pos = (%.2f, %.2f)\n", window, xpos, ypos);

    __mouse_pos[0] = xpos;
    __mouse_pos[1] = ypos;
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

static void render_callback()
{
    // hud
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.01f, 0.0f, -1.0f);
    glVertex3f(0.01f, 0.0f, -1.0f);
    glVertex3f(0.0f, -0.01f, -1.0f);
    glVertex3f(0.0f, 0.01f, -1.0f);
    glEnd();

    // camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // deprecated but multi-platform and functional
    gluPerspective(45, __engine_ctx.window_width / (double)__engine_ctx.window_height, 0.01, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslated(__camera_pos[0], __camera_pos[1], __camera_pos[2]);
    glRotated(__camera_rotation[0], 0.0, 1.0, 0.0);
    glRotated(__camera_rotation[1], 1.0, 0.0, 0.0);
}

static void postrender_callback()
{
}

static void __update_camera_movement(double delta, int axis)
{
    if (__camera_movement_direction[axis] != 0)
    {
	__camera_pos[axis] += delta * __camera_movement_inc[axis] * __camera_movement_direction[axis];
    }
}

static void __update_camera_rotation_direction(double mouse_delta, int axis)
{
    if (mouse_delta > 10.0)
    {
	__camera_rotation_direction[axis] = 1.0;
    }
    else if (mouse_delta < -10.0)
    {
	__camera_rotation_direction[axis] = -1.0;
    }
    else
    {
	__camera_rotation_direction[axis] = 0.0;
    }
}

static void __update_camera_rotation(double delta, int axis)
{
    __update_camera_rotation_direction(__mouse_pos[axis] - __mouse_pos_last_frame[axis], axis);

    if (__camera_rotation_direction[axis] != 0)
    {
	__camera_rotation[axis] += delta * __camera_rotation_inc[axis] * __camera_rotation_direction[axis];
	if (__camera_rotation[axis] < -360.0)
	{
	    __camera_rotation[axis] += 360.0;
	}
	else if (__camera_rotation[axis] > 360.0)
	{
	    __camera_rotation[axis] -= 360.0;
	}

	//LOG_DEBUG("camera rot = (%f, %f)\n", __camera_rotation[0], __camera_rotation[1]);
    }

    __mouse_pos_last_frame[axis] = __mouse_pos[axis];
}

static void update_callback(double delta)
{
    // update camera
    __update_camera_movement(delta, 0);
    __update_camera_movement(delta, 2);
    __update_camera_rotation(delta, 0);
    __update_camera_rotation(delta, 1);

    // rotate cubes
    /*
    for (unsigned long idx = 0; idx < __cubes.len; ++idx)
    {
        render_ctx_t * ctx = (render_ctx_t *) array_get(&__cubes, idx);
        ctx->rotation_angle += delta * 5.0f;
    }
    */
}

static void setup_scene(void)
{
    if (!setup_lighting()) return;

    if (!add_objects_to_scene()) return;
}

static int setup_lighting(void)
{
    GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    //GLfloat mat_emission[] = { 0.1, 0.1, 0.1, 1.0 }; // makes all materials same color
    GLfloat mat_shininess[] = { 128.0 };
    GLfloat light0_position[] = { 1.0, 1.0, 1.0, 0.0 };
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    //glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position); // TODO need more lights?

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    return 1;
}

static int add_objects_to_scene(void)
{
    srand(time(0));

    array_init(&__cubes);
    atexit(remove_objects_from_scene);
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
	    ctx->polygon_mode = (y % 2 == 0 ? GL_LINE : GL_FILL);
            if (array_push(&__cubes, ctx) != status_success) return 0;
    
            if (render_add_object(ctx) != status_success) return 0;
        } 
    }

    return 1;
}

static void remove_objects_from_scene(void)
{
    array_destroy(&__cubes);
}

