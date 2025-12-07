#include "process.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Process *get_processes(DIR** procdir, struct dirent** nextprocdir, int *out_count) {
  Process *processes = NULL;
  int count = 0;
  size_t capacity = 0;

  long total_memory = get_total_memory();

  DIR* proc = *procdir;
  struct dirent* nextproc = *nextprocdir;
  while (nextproc != NULL) {
    char *dir_name = nextproc->d_name;

    if (check_if_process(dir_name)) {
      char stat_str[4096];
      char *fields[60];

      FILE* statfile = get_stat_file(dir_name);
      if (statfile == NULL) {
        nextproc = readdir(proc);
        continue;
      }

      // write stat file to stat_str and tokenize it
      fgets(stat_str, sizeof(stat_str), statfile);
      tokenize_data(stat_str, fields);

      // fill process struct with process information
      Process proc;
      proc.pid = atoi(fields[0]);
      strncpy(proc.name, fields[1], sizeof(proc.name));
      proc.state = fields[2][0];
      proc.ppid = atoi(fields[3]);

      proc.utime = strtoul(fields[13], NULL, 10);
      proc.stime = strtoul(fields[14], NULL, 10);

      proc.priority = atol(fields[17]);
      proc.nice = atol(fields[18]);

      proc.vsize = strtoull(fields[22], NULL, 10);

      long rss = atol(fields[23]) * 4;
      proc.mem_percent = ((float)rss / total_memory) * 100;
      
      // push it to process array
      add_process(&processes, &count, &capacity, proc);

      fclose(statfile);
    }

    nextproc = readdir(proc);
  }

  *out_count = count;

  return processes;
}

void add_process(Process **proc_arr, int *count, size_t *capacity, Process p) {
  if (*count >= *capacity) {
    *capacity = (*capacity == 0) ? 64 : (*capacity * 2);
    *proc_arr = realloc(*proc_arr, (*capacity) * sizeof(Process));
    if (*proc_arr == NULL) {
      printf("realloc failed\n");
      exit(EXIT_FAILURE);
    }
  }

  (*proc_arr)[(*count)++] = p;
}

long get_total_memory() {
  char line[256];
  FILE *memory_file = fopen("/proc/meminfo", "r");
  if (!memory_file) return 0;

  fgets(line, sizeof(line), memory_file);
  fclose(memory_file);

  long total_memory;
  sscanf(line, "MemTotal: %ld kB", &total_memory);

  return total_memory;
}

void tokenize_data(char *stat_str, char **fields) {
  char *first_open_bracket = strchr(stat_str, '(');
  char *last_close_bracket = strrchr(stat_str, ')');
  if (first_open_bracket == NULL || last_close_bracket == NULL) return;


  // manually put delimiter before and after the parentheses
  *first_open_bracket = '\0';
  *last_close_bracket = '\0';

  // set first and second elements to pid and process name
  fields[0] = stat_str;
  fields[1] = first_open_bracket + 1;

  char *remaining_data = last_close_bracket + 2;

  char *token = strtok(remaining_data, " ");

  int i = 2;
  while (token != NULL && i < 52) {
    fields[i++] = token;
    token = strtok(NULL, " ");
  }
}

bool check_if_process(char *dir_name) {
  int i = 0;
  while (dir_name[i] != '\0') {
    if (!isdigit(dir_name[i])) return false;
    i++;
  }

  return true;
}

FILE *get_stat_file(char *pid) {
  char statfile_path[20];
  snprintf(statfile_path, sizeof(statfile_path), "/proc/%s/stat", pid);
  return fopen(statfile_path, "r");
}
