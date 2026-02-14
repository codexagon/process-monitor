#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "display.h"
#include "process.h"
#include "sysinfo.h"

int setup_terminal(struct termios *original_config, struct winsize *win);
int restore_terminal(struct termios *original_config);

typedef enum AppState { STATE_NORMAL, STATE_CONFIRM_KILL } State;

int main() {
	bool running = true;
	State state = STATE_NORMAL;

	struct winsize win;
	struct termios original_config;

	if (setup_terminal(&original_config, &win) != 0)
		return 1;

	fflush(stdout);

	SystemInfo sysinfo = {0};
	SystemInfo syscopy = sysinfo;

	int start_process = 0;
	int max_processes = win.ws_row - 6;

	ProcessList processes = {0};
	ProcessList copy = processes;

	int iteration = 0;

	while (running) {
		int c = getchar();

		// handle keyboard input
		switch (state) {
		case STATE_NORMAL: {
			if (c == 'q' || c == 'Q') {
				running = false;
				break;
			} else if (c == 'k') {
				display_message("Confirm termination of this process (y/n)", win.ws_row);
				state = STATE_CONFIRM_KILL;
			} else if (c == '\033') {
				int next1 = getchar();
				int next2 = getchar();

				if (next1 == '[') {
					if (next2 == 'A') {
						if (processes.selected > 0) {
							processes.selected--;
							if (processes.selected < start_process) {
								start_process--;
							}
						}
					} else if (next2 == 'B') {
						if (processes.selected < (int)processes.count - 1) {
							processes.selected++;
							if (processes.selected >= start_process + max_processes) {
								start_process++;
							}
						}
					}
				}
			}
			break;
		}
		case STATE_CONFIRM_KILL: {
			if (c == 'y' || c == 'Y') {
				signal_process(&processes, SIGTERM);
				display_message("Terminated.", win.ws_row);
				state = STATE_NORMAL;
			} else if (c == 'n' || c == 'N' || c == '\033') {
				display_message("Termination cancelled.", win.ws_row);
				state = STATE_NORMAL;
			}
			break;
		}
		}

		DIR *proc = opendir("/proc");
		if (proc == NULL)
			return 1;

		struct dirent *nextproc = readdir(proc);

		iteration++;
		if (iteration >= 40) {
			syscopy = copy_system_info(&sysinfo);
			if (get_system_info(&sysinfo, &syscopy) != 0)
				return 2;

			copy = copy_process_list(&processes);
			free(processes.processes);
			processes = get_processes(&proc, &nextproc, &copy, &sysinfo);
			free(copy.processes);
			iteration = 0;
		}

		// print processes
		printf("\033[0;0H");
		display_system_info(sysinfo);
		printf("\n");
		display_header(win.ws_col);
		display_processes(&processes, start_process, max_processes);
		fflush(stdout);

		closedir(proc);

		usleep(25000);
	}

	free(processes.processes);

	restore_terminal(&original_config);
	fflush(stdout);
}

int setup_terminal(struct termios *original_config, struct winsize *win) {
	tcgetattr(STDIN_FILENO, original_config);
	struct termios raw_config = *original_config;

	raw_config.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw_config);

	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, win) != 0)
		return 2;

	printf("\033[?25l");     // hide cursor
	printf("\033[?1049h");   // enter alternate buffer
	printf("\033[2J\033[H"); // clear screen and move cursor to home

	return 0;
}

int restore_terminal(struct termios *original_config) {
	tcsetattr(STDIN_FILENO, TCSANOW, original_config);

	printf("\033[2J\033[H"); // clear screen, move cursor to home
	printf("\033[?1049l");   // exit alternate buffer
	printf("\033[?25h");     // show cursor

	return 0;
}
