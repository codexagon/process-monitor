#include "display.h"

void display_header(int term_cols) {
  printf("\033[42m\033[30m%6s  %-50s %-1s %4s %4s %6s %6s %10s %10s %15s", 
    "PID", "NAME", "S", "PRI", "NC", "PPID", "MEM%", "UTIME", "STIME", "VSIZE"
  );
  for (int i = 119; i < term_cols; i++) printf(" ");
  printf("\033[0m\n");
}

void display_processes(ProcessList *list, int start, int max) {
  int end = (max > 0 && start + max < (int)list->count) ? start + max : (int)list->count;

  for (int i = start; i < end; i++) {
    Process *p = &((list->processes)[i]);

    char *state_color;
    char *nice_color;
    char *mem_color;

    if (p->state == 'R') {
      state_color = COLOR_GREEN;
    } else if (p->state == 'Z') {
      state_color = COLOR_RED;
    } else if (p->state == 'I') {
      state_color = COLOR_YELLOW;
    } else if (p->state == 'S') {
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

    if (p->mem_percent == 0) {
      mem_color = COLOR_GRAY;
    } else {
      mem_color = COLOR_RESET;
    }
    
    printf("%6i  %-50s %s%1c%s %4li %s%4li%s %6i %s%6.2f%s %10lu %10lu  %15llu\n",
      p->pid, p->name, 
      state_color, p->state, COLOR_RESET, 
      p->priority, 
      nice_color, p->nice, COLOR_RESET, 
      p->ppid, 
      mem_color, p->mem_percent, COLOR_RESET, 
      p->utime, p->stime, 
      p->vsize
    );
  }
}
