#include "helper.h"

void convert_units(char *buffer, size_t bufsize, unsigned long long size, bool decimals) {
	if (size >= 1024 * 1024 * 1024) {
		if (decimals)
			snprintf(buffer, bufsize, "%.2fG", ((float)size / (1024 * 1024 * 1024)));
		else
			snprintf(buffer, bufsize, "%iG", (int)(size / (1024 * 1024 * 1024)));
	} else if (size >= 1024 * 1024) {
		if (decimals)
			snprintf(buffer, bufsize, "%.2fM", ((float)size / (1024 * 1024)));
		else
			snprintf(buffer, bufsize, "%iM", (int)(size / (1024 * 1024)));
	} else if (size >= 1024) {
		if (decimals)
			snprintf(buffer, bufsize, "%.2fK", ((float)size / 1024));
		else
			snprintf(buffer, bufsize, "%iK", (int)(size / 1024));
	} else {
		if (decimals)
			snprintf(buffer, bufsize, "%.2fB", (float)size);
		else
			snprintf(buffer, bufsize, "%iB", (int)size);
	}
}
