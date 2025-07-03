#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>

extern float aspect_ratio;
extern float fov_rad;

bool renderer_init(void);
void renderer_cleanup(void);
void render_frame(void);

#endif // RENDERER_H