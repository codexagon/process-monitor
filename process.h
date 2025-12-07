#ifndef PROCESS_H
#define PROCESS_H

#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
  int pid;
  char name[256];
  char state;
  int ppid;
  
  unsigned long utime;
  unsigned long stime;

  long priority;
  long nice;

  unsigned long long vsize;
  float mem_percent;
} Process;

Process *get_processes(DIR** procdir, struct dirent** nextprocdir, int *out_count);
void add_process(Process **proc_arr, int *count, size_t *capacity, Process p);
long get_total_memory();
void tokenize_data(char *stat_str, char **fields);
bool check_if_process(char *dir_name);
FILE *get_stat_file(char *pid);

#endif