#include <linux/perf_event.h>
#include <stdio.h>
#include <unistd.h>

#include "perf.h"

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

int perf_has_sufficient_privilege() {
}
