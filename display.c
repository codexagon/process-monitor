#include "display.h"

void display_header(int term_cols) {
  printf("%6s  %-40s %-6s %8s %10s %10s %8s %15s %12s\n", "PID", "NAME", "STATE", "PPID", "UTIME", "STIME", "NICE", "VSIZE", "RSS");
  for (int c = 0; c < term_cols; c++) {
    printf("-");
  }
  printf("\n");
}

void display_processes(Process *processes, int count, int start, int max) {
  int end = (max > 0 && start + max < count) ? start + max : count;

  for (int i = start; i < end; i++) {
    Process *p = &(processes[i]);

    char *state_color;
    char *nice_color;

    if (p->state == 'R') {
      state_color = COLOR_GREEN;
    } else if (p->state == 'Z') {
      state_color = COLOR_RED;
    } else if (p->state == 'I') {
      state_color = COLOR_YELLOW;
    } else if (p->state = 'S') {
      state_color = COLOR_GRAY;
    } else {
      state_color = COLOR_RESET;
    }

    if (p->nice < 0) {
      nice_color = COLOR_RED;
    } else if (p->nice > 0) {
      nice_color = COLOR_GREEN;
    } else {
      nice_color = COLOR_GRAY;
    }
    
    printf("%6i  %-40s %s%6c%s %8i %10lu %10lu %s%8li%s %15llu %12li\n",
      p->pid, p->name, state_color, p->state, COLOR_RESET, p->ppid, p->utime, p->stime, nice_color, p->nice, COLOR_RESET, p->vsize, p->rss
    );
  }
}