#ifndef SYSINFO_H
#define SYSINFO_H

typedef struct {
	long total_memory;
	long free_memory;
	long available_memory;

	long total_swap;
	long free_swap;

	unsigned long long total_cpu_time;
	unsigned long long prev_cpu_time;

	unsigned long long uptime;
	long procs_count;
	int procs_running;

	int cores;
} SystemInfo;

int get_system_info(SystemInfo *info, SystemInfo *copy);
SystemInfo copy_system_info(SystemInfo *info);

#endif
