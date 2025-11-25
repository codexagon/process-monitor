#include "display.h"

void display_header(int term_cols) {
  printf("%6s  %-40s %-6s %8s %10s %10s %8s %15s %12s\n", "PID", "NAME", "STATE", "PPID", "UTIME", "STIME", "NICE", "VSIZE", "RSS");
  for (int c = 0; c < term_cols; c++) {
    printf("-");
  }
  printf("\n");
}

void display_processes(Process **processes, int count, int start, int max) {
  int end = (max > 0 && start + max < count) ? start + max : count;

  for (int i = start; i < end; i++) {
    Process *p = &(*processes)[i];
    
    printf("%6i %-40s %6c %8i %10lu %10lu %8li %15llu %12li\n",
      p->pid, p->name, p->state, p->ppid, p->utime, p->stime, p->nice, p->vsize, p->rss
    );
  }
}