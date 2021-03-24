#ifndef PERF_UTILITIES_H
#define PERF_UTILITIES_H

#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "perf.h"

// An IO error occured. Use errno to gather more information
#define PERF_ERROR_IO -1
// A call to a library method failed. Use errno to gather more information
#define PERF_ERROR_LIBRARY_FAILURE -2
// A capability is not supported.
#define PERF_ERROR_CAPABILITY_NOT_SUPPORTED -3
// A call to perf_event_open failed
#define PERF_ERROR_EVENT_OPEN -4
// Bad parameters received
#define PERF_ERROR_BAD_PARAMETERS -5
// The event is not supported, or invalid
#define PERF_ERROR_NOT_SUPPORTED -6

typedef struct {
  // The attribute for the measurement
  perf_event_attr_t attribute;
  // The file descriptor of the measurement
  int file_descriptor;
  // The PID to measure. 0 for the current process
  pid_t pid;
  // The CPU to measure. -1 for all CPUs
  int cpu;
  // The group the measurement is part of. -1 if parent of a group
  int group;
  // The ID of the measurement
  uint64_t id;
} perf_measurement_t;

// Returns whether or not the perf API is supported.
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

// Prints an error.
void perf_print_error(int error);

// Reads the currently configured event paranoia.
// Returns <0 if an error occured, a PERF_EVENT_PARANOIA_ value otherwise.
// Note: does not return the actually configured paranoia value.
int perf_get_event_paranoia();

// Returns whether or not the current user has sufficient privilege for using the
// perf API.
// Returns <0 if an error occured.
int perf_has_sufficient_privilege(const perf_measurement_t *measurement);

// Returns whether or not the calling user has a specific capability.
// Returns <0 if an error occured.
int perf_has_capability(int capability);

// Create a measurement. Should be freed.
// pid == 0 and cpu == -1 This measures the calling process / thread on any CPU.
// pid == 0 and cpu >= 0 This measures the calling process / thread only when running on the specified CPU.
// pid > 0 and cpu == -1 This measures the specified process / thread on any CPU.
// pid > 0 and cpu >= 0 This measures the specified process / thread only when running on the specified CPU.
// pid == -1 and cpu >= 0 This measures all processes / threads on the specified CPU.This requires CAP_PERFMON(since Linux 5.8) or CAP_SYS_ADMIN capability or a event paranoia value of less than 1.
// pid  == -1 and cpu == -1 This setting is invalid and will return an error.
// Returns NULL if an error occured.
perf_measurement_t *perf_create_measurement(int type, int config, pid_t pid, int cpu);

// Open a measurement to prepare it for usage.
// An opened measurement should be closed using perf_close_measurement.
// Returns <0 if an error occured.
int perf_open_measurement(perf_measurement_t *measurement, int group, int flags);

// Start a measurement. Resets the counter and starts it.
#define perf_start_measurement(measurement)                                          \
  do {                                                                               \
    ioctl(measurement->file_descriptor, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);  \
    ioctl(measurement->file_descriptor, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP); \
  } while (0)

// Stop a measurement.
#define perf_stop_measurement(measurement) ioctl(measurement->file_descriptor, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP)

// Read a measured value.
// Return the number read, -1 for errors or 0 for EOF.
int perf_read_measurement(const perf_measurement_t *measurement, void *target, size_t bytes);

// Get the kernel. Use NULL to ignore a value.
// Returns <0 if an error occured.
int perf_get_kernel_version(int *major, int *minor, int *patch);

// Whether or not an event is supported. Creates an event and then closes it immediately.
// Returns <0 if an error occured.
int perf_event_is_supported(const perf_measurement_t *measurement);

// Close the measurement.
// Returns <0 if an error occured.
int perf_close_measurement(const perf_measurement_t *measurement);

#endif
