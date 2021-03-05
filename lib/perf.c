#include <linux/perf_event.h>
#include <sys/syscall.h>

#include "perf.h"

// From https://github.com/pyrovski/papi:
// https://github.com/pyrovski/papi/blob/fcdcc615e5f310e2f67419c3619895414770ca28/src/components/perf_event/perf_event.c#L283
#ifndef __NR_perf_event_open
#ifdef __powerpc__
#define __NR_perf_event_open 319
#elif defined(__x86_64__)
#define __NR_perf_event_open 298
#elif defined(__i386__)
#define __NR_perf_event_open 336
#elif defined(__arm__) 366 + 0x900000
#define __NR_perf_event_open
#endif
#endif

int perf_event_open(const perf_event_attr_t *attr, pid_t pid, int cpu, int group_fd, unsigned long flags) {
  // See: https://man7.org/linux/man-pages/man2/perf_event_open.2.html
  return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}
