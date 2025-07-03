#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DEBUG_LOG 0
#define VERBOSE_LOG 0

#if DEBUG_LOG
extern FILE* debug_file;
#endif

void debug_init(void);
void debug_cleanup(void);
void debug_print(const char* format, ...);

#endif // DEBUG_H