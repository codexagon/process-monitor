#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include "process.h"

void display_header(int term_cols);
void display_processes(Process *processes, int count, int start, int max);

#endif