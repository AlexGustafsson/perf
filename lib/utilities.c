#include <errno.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "perf.h"
#include "utilities.h"

// CAP_PERFMON was added in Linux 5.8
// https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/capability.h
#ifndef CAP_PERFMON
#define CAP_PERFMON 38
#endif

#define true 1
#define false 0

void perf_print_error(int error) {
  switch (error) {
  case PERF_ERROR_IO:
    perror("io error");
    break;
  case PERF_ERROR_LIBRARY_FAILURE:
    perror("library failure");
    break;
  case PERF_ERROR_CAPABILITY_NOT_SUPPORTED:
    fprintf(stderr, "unsupported capability\n");
    break;
  case PERF_ERROR_EVENT_OPEN:
    perror("perf_event_open failed");
    break;
  case PERF_ERROR_NOT_SUPPORTED:
    perror("not supported");
    break;
  case PERF_ERROR_BAD_PARAMETERS:
    fprintf(stderr, "bad parameters\n");
    break;
  default:
    fprintf(stderr, "unknown error\n");
    break;
  }
}

int perf_is_supported() {
  return access("/proc/sys/kernel/perf_event_paranoid", F_OK) == 0 ? 1 : 0;
}

int perf_get_event_paranoia() {
  // See: https://www.kernel.org/doc/Documentation/sysctl/kernel.txt
  FILE *perf_event_paranoid = fopen("/proc/sys/kernel/perf_event_paranoid", "r");
  if (perf_event_paranoid == NULL)
    return PERF_ERROR_IO;

  int value;
  if (fscanf(perf_event_paranoid, "%d", &value) < 1)
    return PERF_ERROR_IO;

  if (value >= 2)
    return PERF_EVENT_PARANOIA_DISALLOW_CPU | PERF_EVENT_PARANOIA_DISALLOW_FTRACE | PERF_EVENT_PARANOIA_DISALLOW_KERNEL;
  if (value >= 1)
    return PERF_EVENT_PARANOIA_DISALLOW_CPU | PERF_EVENT_PARANOIA_DISALLOW_FTRACE;
  if (value >= 0)
    return PERF_EVENT_PARANOIA_DISALLOW_CPU;
  return PERF_EVENT_PARANOIA_ALLOW_ALL;
}

int perf_has_sufficient_privilege(const perf_measurement_t *measurement) {
  // Immediately return if the user is an admin
  int has_cap_sys_admin = perf_has_capability(CAP_SYS_ADMIN);
  if (has_cap_sys_admin == 1)
    return true;

  int event_paranoia = perf_get_event_paranoia();
  if (event_paranoia < 0)
    return event_paranoia;

  // This requires CAP_PERFMON (since Linux 5.8) or CAP_SYS_ADMIN capability or a  perf_event_paranoid value of less than 1.
  if (measurement->pid == -1 && measurement->cpu >= 0) {
    int kernel_major, kernel_minor;
    int status = perf_get_kernel_version(&kernel_major, &kernel_minor, NULL);
    if (status != true)
      return status;

    if (kernel_major >= 5 && kernel_minor >= 8) {
      int has_cap_perfmon = perf_has_capability(CAP_PERFMON);
      if (has_cap_perfmon != true)
        return has_cap_perfmon;
    } else {
      // CAP_SYS_ADMIN is already checked at the start of this function
    }
  }

  // Immediately return if all events are allowed
  if (event_paranoia & PERF_EVENT_PARANOIA_ALLOW_ALL)
    return true;

  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_FTRACE && measurement->attribute.type == PERF_TYPE_TRACEPOINT)
    return has_cap_sys_admin;

  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_CPU && measurement->attribute.type == PERF_TYPE_HARDWARE)
    return has_cap_sys_admin;

  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_KERNEL && measurement->attribute.type == PERF_TYPE_SOFTWARE)
    return has_cap_sys_admin;

  // Assume privileged
  return true;
}

int perf_has_capability(int capability) {
  if (!CAP_IS_SUPPORTED(capability))
    return PERF_ERROR_CAPABILITY_NOT_SUPPORTED;

  cap_t capabilities = cap_get_proc();
  if (capabilities == NULL)
    return PERF_ERROR_LIBRARY_FAILURE;

  cap_flag_value_t sys_admin_value;
  if (cap_get_flag(capabilities, capability, CAP_EFFECTIVE, &sys_admin_value) < 0) {
    cap_free(capabilities);
    return PERF_ERROR_LIBRARY_FAILURE;
  }

  if (cap_free(capabilities) < 0)
    return PERF_ERROR_LIBRARY_FAILURE;

  // Return whether or not the user has the capability
  return sys_admin_value == CAP_SET;
}

perf_measurement_t *perf_create_measurement(int type, int config, pid_t pid, int cpu) {
  perf_measurement_t *measurement = (perf_measurement_t *)malloc(sizeof(perf_measurement_t));
  if (measurement == NULL)
    return NULL;

  memset((void *)measurement, 0, sizeof(perf_measurement_t));

  measurement->pid = pid;
  measurement->cpu = cpu;

  measurement->attribute.type = type;
  measurement->attribute.config = config;
  measurement->attribute.disabled = 1;
  measurement->attribute.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  return measurement;
}

int perf_open_measurement(perf_measurement_t *measurement, int group, int flags) {
  // Invalid parameters. See: https://man7.org/linux/man-pages/man2/perf_event_open.2.html
  if (measurement->pid == -1 && measurement->cpu == -1)
    return PERF_ERROR_BAD_PARAMETERS;

  int file_descriptor = perf_event_open(&measurement->attribute, measurement->pid, measurement->cpu, group, flags);
  if (file_descriptor < 0) {
    if (errno == ENODEV ||
        errno == ENOENT ||
        errno == ENOSYS ||
        errno == EOPNOTSUPP ||
        errno == EPERM)
      return PERF_ERROR_NOT_SUPPORTED;
    return PERF_ERROR_EVENT_OPEN;
  }

  measurement->file_descriptor = file_descriptor;
  measurement->group = group;

  // Get the ID of the measurement
  if (ioctl(measurement->file_descriptor, PERF_EVENT_IOC_ID, &measurement->id) < 0)
    return PERF_ERROR_LIBRARY_FAILURE;

  return 0;
}

int perf_read_measurement(const perf_measurement_t *measurement, void *target, size_t bytes) {
  return read(measurement->file_descriptor, target, bytes);
}

int perf_get_kernel_version(int *major, int *minor, int *patch) {
  struct utsname name;
  if (uname(&name) < 0)
    return PERF_ERROR_LIBRARY_FAILURE;

  int parsed_major, parsed_minor, parsed_patch;

  if (sscanf(name.release, "%d.%d.%d", &parsed_major, &parsed_minor, &parsed_patch) < 3)
    return PERF_ERROR_IO;

  if (major != NULL)
    *major = parsed_major;
  if (minor != NULL)
    *minor = parsed_minor;
  if (patch != NULL)
    *patch = parsed_patch;

  return 0;
}

int perf_event_is_supported(const perf_measurement_t *measurement) {
  // Invalid parameters. See: https://man7.org/linux/man-pages/man2/perf_event_open.2.html
  if (measurement->pid == -1 && measurement->cpu == -1)
    return PERF_ERROR_BAD_PARAMETERS;

  int file_descriptor = perf_event_open(&measurement->attribute, measurement->pid, measurement->cpu, -1, 0);
  if (file_descriptor < 0) {
    if (errno == ENODEV ||
        errno == ENOENT ||
        errno == ENOSYS ||
        errno == EOPNOTSUPP ||
        errno == EPERM)
      return 0;
    return PERF_ERROR_EVENT_OPEN;
  }

  if (close(file_descriptor) < 0)
    return PERF_ERROR_IO;

  return 1;
}

int perf_close_measurement(const perf_measurement_t *measurement) {
  if (close(measurement->file_descriptor) < 0)
    return PERF_ERROR_IO;

  return 0;
}
