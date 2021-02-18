#ifndef LIB_H
#define LIB_H

#include <linux/perf_event.h> // "Convenience include"
#include <unistd.h>

// Checks whether or not the perf API is supported.
// Returns 1 if there is support and 0 otherwise.
int perf_is_supported();

// -1 : Allow use of (almost) all events by all users. Ignore mlock limit after
// perf_event_mlock_kb without CAP_IPC_LOCK.
#define PERF_EVENT_PARANOIA_ALLOW_ALL (1 << 0)
// >=0 : Disallow ftrace function tracepoint by users without CAP_SYS_ADMIN.
// Disallow raw tracepoint access by users without CAP_SYS_ADMIN.
#define PERF_EVENT_PARANOIA_DISALLOW_FTRACE (1 << 1)
// >=1 : Disallow CPU event access by users without CAP_SYS_ADMIN.
#define PERF_EVENT_PARANOIA_DISALLOW_CPU (1 << 2)
// >=2 : Disallow kernel profiling by users without CAP_SYS_ADMIN.
#define PERF_EVENT_PARANOIA_DISALLOW_KERNEL (1 << 3)

// Reads the currently configured event paranoia.
// Returns -1 if an error occured, a PERF_EVENT_PARANOIA_ value otherwise.
// Note: does not return the actually configured paranoia value.
int perf_get_event_paranoia();
// Checks whether the current user has sufficient privilege for using the
// perf API. Returns 1 if there user has sufficient privileges, 0 if not and
// -1 if there was an error.
int perf_has_sufficient_privilege(int event_paranoia);

// Wrapper for perf_event_open(2). You likely want to use a utility method instead.
int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags);

#endif
