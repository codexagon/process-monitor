#include "display.h"
#include "helper.h"

void display_system_info(SystemInfo info) {
	char used_mem_str[16];
	convert_units(used_mem_str, sizeof(used_mem_str), (info.total_memory - info.free_memory) * 1024, true);

	char total_mem_str[16];
	convert_units(total_mem_str, sizeof(total_mem_str), info.total_memory * 1024, true);

	printf("Memory: %s/%s used\n", used_mem_str, total_mem_str);

	char used_swap_str[16];
	convert_units(used_swap_str, sizeof(used_swap_str), (info.total_swap - info.free_swap) * 1024, true);

	char total_swap_str[16];
	convert_units(total_swap_str, sizeof(total_swap_str), info.total_swap * 1024, true);

	printf("Swap: %s/%s used\n", used_swap_str, total_swap_str);

	int hours = (int)(info.uptime / 3600);
	int minutes = (int)((info.uptime - (hours * 3600)) / 60);
	printf("Uptime: %i hours, %i minutes   Processes: %ld\n", hours, minutes, info.procs_count);
}

void display_header(int term_cols) {
	// clang-format off
	printf(
		"\033[42m\033[30m%6s  %-50s %-1s %4s %4s %6s %6s %6s %6s", 
		"PID", "NAME", "S", "PRI", "NC", "PPID", "MEM%", "CPU%", "VSIZE"
	);
	// clang-format on

	for (int i = 98; i < term_cols; i++)
		printf(" ");

	printf("\033[0m\n");
}

void display_processes(ProcessList *list, int start, int max) {
	int end = (max > 0 && start + max < (int)list->count) ? start + max : (int)list->count;

	for (int i = start; i < end; i++) {
		Process *p = &((list->processes)[i]);

		char *state_color;
		char *nice_color;
		char *mem_color;
		char *cpu_color;

		if (p->state == 'R') {
			state_color = COLOR_GREEN;
		} else if (p->state == 'Z') {
			state_color = COLOR_RED;
		} else if (p->state == 'I') {
			state_color = COLOR_YELLOW;
		} else if (p->state == 'S') {
			state_color = COLOR_GRAY;
		} else {
			state_color = COLOR_RESET;
		}

		if (p->nice < 0) {
			nice_color = COLOR_RED;
		} else if (p->nice > 0) {
			nice_color = COLOR_GREEN;
		} else {
			nice_color = COLOR_GRAY;
		}

		if (p->mem_percent == 0) {
			mem_color = COLOR_GRAY;
		} else {
			mem_color = COLOR_RESET;
		}

		if (p->cpu_percent == 0) {
			cpu_color = COLOR_GRAY;
		} else {
			cpu_color = COLOR_RESET;
		}

		if (i == list->selected) {
			// clang-format off
			printf("\033[30;46m%6i  %-50s %1c %4li %4li %6i %6.2f %6.2f %6s\033[0m\n",
				p->pid, p->name, p->state, p->priority, p->nice, p->ppid, p->mem_percent, p->cpu_percent, p->vsize_str
			);
			// clang-format on
		} else {
			// clang-format off
			printf("%6i  %-50s %s%1c%s %4li %s%4li%s %6i %s%6.2f%s %s%6.2f%s %6s\n",
				p->pid, p->name, 
				state_color, p->state, COLOR_RESET, 
				p->priority, 
				nice_color, p->nice, COLOR_RESET, 
				p->ppid, 
				mem_color, p->mem_percent, COLOR_RESET, 
				cpu_color, p->cpu_percent, COLOR_RESET, 
				p->vsize_str
			);
			// clang-format on
		}
	}
}
