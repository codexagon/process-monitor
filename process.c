#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "process.h"

ProcessList get_processes(DIR **procdir, struct dirent **nextprocdir, ProcessList *cur_list) {
	ProcessList process_list = {0};

	long total_memory = get_total_memory();
	int cores = sysconf(_SC_NPROCESSORS_ONLN);

	process_list.prev_cpu_time = cur_list->total_cpu_time;
	process_list.total_cpu_time = get_cpu_time();

	DIR *proc = *procdir;
	struct dirent *nextproc = *nextprocdir;
	while (nextproc != NULL) {
		char *dir_name = nextproc->d_name;

		if (check_if_process(dir_name)) {
			char stat_str[4096];
			char *fields[60];

			FILE *statfile = get_stat_file(dir_name);
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

			proc.utime = strtoull(fields[13], NULL, 10);
			proc.stime = strtoull(fields[14], NULL, 10);

			proc.prev_utime = proc.utime;
			proc.prev_stime = proc.stime;
			for (int i = 0; i < (int)cur_list->count; i++) {
				if ((cur_list->processes)[i].pid == proc.pid) {
					proc.prev_utime = (cur_list->processes)[i].utime;
					proc.prev_stime = (cur_list->processes)[i].stime;
					break;
				}
			}

			proc.cpu_percent = ((double)(proc.utime + proc.stime - proc.prev_utime - proc.prev_stime) /
			                    (double)(process_list.total_cpu_time - process_list.prev_cpu_time)) *
			                   100 * cores;

			proc.priority = atol(fields[17]);
			proc.nice = atol(fields[18]);

			proc.vsize = strtoull(fields[22], NULL, 10);
			if (proc.vsize >= 1024 * 1024 * 1024) {
				snprintf(proc.vsize_str, sizeof(proc.vsize_str), "%iG", (int)(proc.vsize / (1024 * 1024 * 1024)));
			} else if (proc.vsize >= 1024 * 1024) {
				snprintf(proc.vsize_str, sizeof(proc.vsize_str), "%iM", (int)(proc.vsize / (1024 * 1024)));
			} else if (proc.vsize >= 1024) {
				snprintf(proc.vsize_str, sizeof(proc.vsize_str), "%iK", (int)(proc.vsize / 1024));
			}

			long rss = atol(fields[23]) * 4;
			proc.mem_percent = ((float)rss / total_memory) * 100;

			// push it to process array
			add_process(&process_list, proc);

			fclose(statfile);
		}

		nextproc = readdir(proc);
	}

	return process_list;
}

void add_process(ProcessList *list, Process p) {
	if (list->count >= list->capacity) {
		list->capacity = (list->capacity == 0) ? 16 : (list->capacity * 2);
		Process *tmp = realloc(list->processes, (list->capacity) * sizeof(Process));
		if (tmp == NULL) {
			printf("Failed to reallocate processes list.\n");
			exit(EXIT_FAILURE);
		}
		list->processes = tmp;
	}

	(list->processes)[(list->count)++] = p;
}

ProcessList copy_process_list(ProcessList *list) {
	ProcessList new_list = {0};

	new_list.prev_cpu_time = list->prev_cpu_time;
	new_list.total_cpu_time = list->total_cpu_time;

	for (int i = 0; i < (int)list->count; i++) {
		Process proc;
		proc.pid = (list->processes)[i].pid;
		strncpy(proc.name, (list->processes)[i].name, sizeof(proc.name));
		proc.state = (list->processes)[i].state;
		proc.ppid = (list->processes)[i].ppid;

		proc.utime = (list->processes)[i].utime;
		proc.stime = (list->processes)[i].stime;
		proc.prev_utime = (list->processes)[i].prev_utime;
		proc.prev_stime = (list->processes)[i].prev_stime;

		proc.cpu_percent = (list->processes)[i].cpu_percent;
		proc.priority = (list->processes)[i].priority;
		proc.nice = (list->processes)[i].nice;

		proc.vsize = (list->processes)[i].vsize;
		proc.mem_percent = (list->processes)[i].mem_percent;

		add_process(&new_list, proc);
	}

	return new_list;
}

long get_total_memory() {
	char line[256];
	FILE *memory_file = fopen("/proc/meminfo", "r");
	if (!memory_file)
		return 0;

	fgets(line, sizeof(line), memory_file);
	fclose(memory_file);

	long total_memory;
	sscanf(line, "MemTotal: %ld kB", &total_memory);

	return total_memory;
}

unsigned long long get_cpu_time() {
	char line[256];
	FILE *stat_file = fopen("/proc/stat", "r");
	if (!stat_file)
		return 1;

	fgets(line, sizeof(line), stat_file);
	fclose(stat_file);

	unsigned long long total_time[10];
	// clang-format off
	sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
			&total_time[0], &total_time[1], &total_time[2], &total_time[3], &total_time[4],
			&total_time[5], &total_time[6], &total_time[7], &total_time[8], &total_time[9]);
	// clang-format on

	unsigned long long sum = 0;
	for (int i = 0; i < 8; i++)
		sum += total_time[i];

	return sum;
}

void tokenize_data(char *stat_str, char **fields) {
	char *first_open_bracket = strchr(stat_str, '(');
	char *last_close_bracket = strrchr(stat_str, ')');
	if (first_open_bracket == NULL || last_close_bracket == NULL)
		return;

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
		if (!isdigit(dir_name[i]))
			return false;
		i++;
	}

	return true;
}

FILE *get_stat_file(char *pid) {
	char statfile_path[20];
	snprintf(statfile_path, sizeof(statfile_path), "/proc/%s/stat", pid);
	return fopen(statfile_path, "r");
}
