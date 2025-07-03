#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <stdbool.h>

bool primitives_init(void);
void primitives_cleanup(void);
void draw_floor(void);
void draw_target(void);
void draw_crosshair(void);

#endif // PRIMITIVES_H