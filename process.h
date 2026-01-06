#ifndef PROCESS_H
#define PROCESS_H

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>

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

typedef struct {
	Process *processes;
	size_t count;
	size_t capacity;
} ProcessList;

ProcessList get_processes(DIR **procdir, struct dirent **nextprocdir);
void add_process(ProcessList *list, Process p);
long get_total_memory();
void tokenize_data(char *stat_str, char **fields);
bool check_if_process(char *dir_name);
FILE *get_stat_file(char *pid);

#endif
