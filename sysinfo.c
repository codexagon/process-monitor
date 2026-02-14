#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sysinfo.h"

int get_system_info(SystemInfo *info, SystemInfo *copy) {
	char line[256];
	info->cores = sysconf(_SC_NPROCESSORS_ONLN);

	FILE *memory_file = fopen("/proc/meminfo", "r");
	if (!memory_file)
		return 1;

	while (fgets(line, sizeof(line), memory_file)) {
		if (sscanf(line, "MemTotal: %ld kB", &(info->total_memory)) == 1)
			continue;
		if (sscanf(line, "MemFree: %ld kB", &(info->free_memory)) == 1)
			continue;
		if (sscanf(line, "MemAvailable: %ld kB", &(info->available_memory)) == 1)
			continue;
		if (sscanf(line, "SwapTotal: %ld kB", &(info->total_swap)) == 1)
			continue;
		if (sscanf(line, "SwapFree: %ld kB", &(info->free_swap)) == 1)
			continue;
	}
	fclose(memory_file);

	FILE *stat_file = fopen("/proc/stat", "r");
	if (!stat_file)
		return 2;

	while (fgets(line, sizeof(line), stat_file)) {
		if (strncmp(line, "cpu ", 4) == 0) {
			unsigned long long total_time[10];
			// clang-format off
			sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
					&total_time[0], &total_time[1], &total_time[2], &total_time[3], &total_time[4],
					&total_time[5], &total_time[6], &total_time[7], &total_time[8], &total_time[9]);
			// clang-format on

			unsigned long long sum = 0;
			for (int j = 0; j < 8; j++)
				sum += total_time[j];

			info->total_cpu_time = sum;
			info->prev_cpu_time = copy->total_cpu_time;
		} else if (sscanf(line, "processes %li", &(info->procs_count)) == 1) {
			continue;
		} else if (sscanf(line, "procs_running %i", &(info->procs_running)) == 1) {
			continue;
		}
	}
	fclose(stat_file);

	FILE *uptime_file = fopen("/proc/uptime", "r");
	if (!uptime_file)
		return 3;

	fgets(line, sizeof(line), uptime_file);
	sscanf(line, "%llu", &(info->uptime));
	fclose(uptime_file);

	return 0;
}

SystemInfo copy_system_info(SystemInfo *info) {
	SystemInfo new_info = {0};

	new_info.total_memory = info->total_memory;
	new_info.free_memory = info->free_memory;
	new_info.available_memory = info->available_memory;

	new_info.total_swap = info->total_swap;
	new_info.free_swap = info->free_swap;

	new_info.total_cpu_time = info->total_cpu_time;
	new_info.prev_cpu_time = info->prev_cpu_time;

	new_info.uptime = info->uptime;
	new_info.procs_count = info->procs_count;
	new_info.procs_running = info->procs_running;

	return new_info;
}
