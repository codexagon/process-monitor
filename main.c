#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

void print_processes(DIR**, struct dirent**, int);
void tokenize_data(char *, char **);
bool check_if_process(char *);
FILE *get_stat_file(char *);

int main() {
  bool running = true;

  struct winsize win;
  struct termios original_config;
  tcgetattr(STDIN_FILENO, &original_config);
  struct termios raw_config = original_config;

  // disable canonical mode & input echoing
  raw_config.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw_config);

  // make non-blocking
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  int success = ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
  if (success != 0) return 2;
  
  int max_processes = win.ws_row - 3;

  printf("\033[?25l"); // hide cursor
  printf("\033[?1049h"); // enter alternate buffer
  printf("\033[2J\033[H"); // clear screen and move cursor to home
  fflush(stdout);

  // print header
  printf("%6s  %-40s %-6s %8s %10s %10s %8s %15s %12s\n", "PID", "NAME", "STATE", "PPID", "UTIME", "STIME", "NICE", "VSIZE", "RSS");
  for (int c = 0; c < win.ws_col; c++) {
    printf("-");
  }
  printf("\n");

  while (running) {
    int c = getchar();

    // handle keyboard input
    if (c == 'q' || c == 'Q') {
      running = false;
      break;
    }

    printf("\033[3H"); // move cursor to line 3, just below header

    DIR* proc = opendir("/proc");
    if (proc == NULL) return 1;

    struct dirent* nextproc = readdir(proc);

    // print processes
    print_processes(&proc, &nextproc, max_processes);
    printf("\033[J"); // clear from cursor to end
    fflush(stdout);

    closedir(proc);
    sleep(1);
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &original_config);
  printf("\033[2J\033[H"); // clear screen, move cursor to home
  printf("\033[?1049l"); // exit alternate buffer
  printf("\033[?25h"); // show cursor
  fflush(stdout);
}

void print_processes(DIR** procdir, struct dirent** nextprocdir, int max_proc) {
  int proc_counter = 0;

  DIR* proc = *procdir;
  struct dirent* nextproc = *nextprocdir;
  while (nextproc != NULL) {
    char *dir_name = nextproc->d_name;

    if (check_if_process(dir_name)) {
      if (proc_counter >= max_proc) return;

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

      // list all processes and some data
      printf("%6s  %-40.40s %-6s %8s %10s %10s %8s %15s %12i\n", fields[0], fields[1], fields[2], fields[3], fields[13], fields[14], fields[19], fields[22], atoi(fields[23]) * 4096);
      proc_counter++;

      fclose(statfile);
    }

    nextproc = readdir(proc);
  }
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
