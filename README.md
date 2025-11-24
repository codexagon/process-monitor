# Process Monitor

A basic process monitor for Linux written in C. Displays real-time process information by reading from the `/proc/` filesystem.

## Building
```bash
gcc -o pmonitor main.c
```

## Usage
```bash
./pmonitor
```

Press `q` or `Q` to quit.

## Note
- Only works on Linux, because the program reads from the `/proc/` filesystem.
