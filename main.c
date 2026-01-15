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

int setup_terminal(struct termios *original_config, struct winsize *win);
int restore_terminal(struct termios *original_config);

int main() {
	bool running = true;

	struct winsize win;
	struct termios original_config;

	if (setup_terminal(&original_config, &win) != 0)
		return 1;

	fflush(stdout);

	int start_process = 0;
	int max_processes = win.ws_row - 2;

	display_header(win.ws_col);

	ProcessList processes = {0};
	ProcessList copy = processes;

	int iteration = 0;

	while (running) {
		int c = getchar();

		// handle keyboard input
		if (c == 'q' || c == 'Q') {
			running = false;
			break;
		} else if (c == '\033') {
			int next1 = getchar();
			int next2 = getchar();

			if (next1 == '[') {
				if (next2 == 'A') {
					if (start_process > 0)
						start_process--;
				} else if (next2 == 'B') {
					if (start_process + max_processes <= (int)processes.count)
						start_process++;
				}
			}
		} else if (c == 'k') {
			if (start_process > 0)
				start_process--;
		} else if (c == 'j') {
			if (start_process + max_processes <= (int)processes.count)
				start_process++;
		}

		printf("\033[2H"); // move cursor to line 2

		DIR *proc = opendir("/proc");
		if (proc == NULL)
			return 1;

		struct dirent *nextproc = readdir(proc);

		iteration++;
		if (iteration >= 40) {
			copy = copy_process_list(&processes);
			free(processes.processes);
			processes = get_processes(&proc, &nextproc, &copy);
			iteration = 0;
		}

		// print processes
		display_processes(&processes, start_process, max_processes);
		printf("\033[J"); // clear from cursor to end
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
