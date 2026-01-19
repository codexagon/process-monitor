CC = gcc
CFLAGS = -Wall -Wextra
TARGET = pmon
SOURCES = main.c process.c display.c sysinfo.c helper.c

build:
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
