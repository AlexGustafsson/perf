#ifndef PERF_UTILITIES_H
#define PERF_UTILITIES_H

#include <unistd.h>
#include <stdint.h>

#include "perf.h"

typedef struct {
  // The attribute for the measurement
  perf_event_attr_t attribute;
  // The file descriptor of the measurement
  int file_descriptor;
} perf_measurement_t;

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

// Create a measurement. Should be freed.
perf_measurement_t *perf_create_measurement(int type, int config);
// Open a measurement to prepare it for usage. Returns 0 for success, -1 otherwise.
int perf_open_measurement(perf_measurement_t *measurement, pid_t pid, int cpu, int group, int flags);
// Start a measurement. Resets the counter and starts it.
#define perf_start_measurement(measurement) do { ioctl(measurement->file_descriptor, PERF_EVENT_IOC_RESET, 0); ioctl(measurement->file_descriptor, PERF_EVENT_IOC_ENABLE, 0); } while(0)
// Stop a measurement.
#define perf_stop_measurement(measurement) ioctl(measurement->file_descriptor, PERF_EVENT_IOC_DISABLE, 0)
// Read a measured value.
int perf_read_measurement(perf_measurement_t *measurement, uint64_t *value);

#endif
