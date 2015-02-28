#ifndef __ENGINE_H__
#define __ENGINE_H__

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

#endif
