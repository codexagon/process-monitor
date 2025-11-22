#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void tokenize_data(char *, char **);
bool check_if_process(char *);
FILE *get_stat_file(char *);

int main() {
  DIR* pdir = opendir("/proc");
  if (pdir == NULL) return 1;

  struct dirent* nextpdir = readdir(pdir);

  // print header
  printf("%6s  %-40s %-6s %8s %10s %10s %8s %15s %12s\n", 
    "PID", "NAME", "STATE", "PPID", "UTIME", "STIME", "NICE", "VSIZE", "RSS");
  printf("----------------------------------------------------------------------------------------------------------------------------\n");

  while (nextpdir != NULL) {
    char *dir_name = nextpdir->d_name;

    if (check_if_process(dir_name)) {
      char stat_str[4096];
      char *fields[60];

      FILE* statfile = get_stat_file(dir_name);
      if (statfile == NULL) {
        nextpdir = readdir(pdir);
        continue;
      }

      // write stat file to stat_str and tokenize it
      fgets(stat_str, sizeof(stat_str), statfile);
      tokenize_data(stat_str, fields);

      // list all processes and some data
      printf("%6s  %-40.40s %-6s %8s %10s %10s %8s %15s %12i\n", fields[0], fields[1], fields[2], fields[3], fields[13], fields[14], fields[19], fields[22], atoi(fields[23]) * 4096);

      fclose(statfile);
    }

    nextpdir = readdir(pdir);
  }

  closedir(pdir);
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
    if (!isdigit(*(dir_name + i))) return false;
    i++;
  }

  return true;
}

FILE *get_stat_file(char *pid) {
  char statfile_path[20];
  snprintf(statfile_path, sizeof(statfile_path), "/proc/%s/stat", pid);
  return fopen(statfile_path, "r");
}
