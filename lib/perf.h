#ifndef PERF_H
#define PERF_H

#include <linux/perf_event.h> // "Convenience include"
#include <unistd.h>

// Wrapper for perf_event_open(2). You likely want to use a utility method instead.
int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags);

#endif
