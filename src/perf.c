#include <linux/perf_event.h>
#include <stdio.h>
#include <sys/capability.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

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

int perf_is_supported() {
  return access("/proc/sys/kernel/perf_event_paranoid", F_OK) == 0 ? 0 : 1;
}

int perf_get_event_paranoia() {
  // See: https://www.kernel.org/doc/Documentation/sysctl/kernel.txt
  FILE *perf_event_paranoid = fopen("/proc/sys/kernel/perf_event_paranoid", "r");
  if (perf_event_paranoid == NULL)
    return -1;

  int value;
  if (fscanf(perf_event_paranoid, "%d", &value) != 1)
    return -1;

  if (value >= 2) return PERF_EVENT_PARANOIA_DISALLOW_CPU | PERF_EVENT_PARANOIA_DISALLOW_FTRACE | PERF_EVENT_PARANOIA_DISALLOW_KERNEL;
  if (value >= 1) return PERF_EVENT_PARANOIA_DISALLOW_CPU | PERF_EVENT_PARANOIA_DISALLOW_FTRACE;
  if (value >= 0) return PERF_EVENT_PARANOIA_DISALLOW_CPU;
  return PERF_EVENT_PARANOIA_ALLOW_ALL;
}

int perf_has_sufficient_privilege(int event_paranoia) {
  // Immediately return if all events are allowed
  if (event_paranoia & PERF_EVENT_PARANOIA_ALLOW_ALL)
    return 1;

  if (!CAP_IS_SUPPORTED(CAP_SYS_ADMIN))
    return -1;

  cap_t capabilities = cap_get_proc();
  if (capabilities == NULL)
    return -1;

  cap_flag_value_t sys_admin_value;
  if (cap_get_flag(capabilities, CAP_SYS_ADMIN, CAP_EFFECTIVE, &sys_admin_value) == -1) {
    cap_free(capabilities);
    return -1;
  }

  if (cap_free(capabilities) == -1)
    return -1;

  // Return whether or not the user has CAP_SYS_ADMIN
  return sys_admin_value == CAP_SET;
}

int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags) {
  // See: https://man7.org/linux/man-pages/man2/perf_event_open.2.html
  return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}
