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

	unsigned long long utime;
	unsigned long long stime;
	unsigned long long prev_utime;
	unsigned long long prev_stime;
	float cpu_percent;

	long priority;
	long nice;

	unsigned long long vsize;
	float mem_percent;
} Process;

typedef struct {
	Process *processes;
	size_t count;
	size_t capacity;
	unsigned long long total_cpu_time;
	unsigned long long prev_cpu_time;
} ProcessList;

ProcessList get_processes(DIR **procdir, struct dirent **nextprocdir, ProcessList *cur_list);
void add_process(ProcessList *list, Process p);
ProcessList copy_process_list(ProcessList *list);
long get_total_memory();
unsigned long long get_cpu_time();
void tokenize_data(char *stat_str, char **fields);
bool check_if_process(char *dir_name);
FILE *get_stat_file(char *pid);

#endif
