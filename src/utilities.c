#include <stdio.h>
#include <sys/capability.h>

#include "perf.h"
#include "utilities.h"

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

  if (value >= 2)
    return PERF_EVENT_PARANOIA_DISALLOW_CPU | PERF_EVENT_PARANOIA_DISALLOW_FTRACE | PERF_EVENT_PARANOIA_DISALLOW_KERNEL;
  if (value >= 1)
    return PERF_EVENT_PARANOIA_DISALLOW_CPU | PERF_EVENT_PARANOIA_DISALLOW_FTRACE;
  if (value >= 0)
    return PERF_EVENT_PARANOIA_DISALLOW_CPU;
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
