#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>

#include "process.h"

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GRAY "\033[90m"
#define COLOR_RESET "\033[0m"

void display_header(int term_cols);
void display_processes(ProcessList *list, int start, int max);

#endif
