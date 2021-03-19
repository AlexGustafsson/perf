#ifndef HARNESS_H
#define HARNESS_H

#include <perf/utilities.h>

#define TEST_ITERATIONS 10

// This structure is returned each time the main measurement is read.
// It contains the result of all the measurements, taken simultaneously.
// The order of the measurements is unknown - used the ID.
typedef struct {
  uint64_t recorded_values;
  struct {
    uint64_t value;
    uint64_t id;
  } values[5];
} measurement_t;

// The main measuring group.
perf_measurement_t *all_measurements;
// Retired instructions. Be careful, these can be affected by various issues, most notably hardware interrupt counts.
perf_measurement_t *measure_instruction_count;
// Total cycles; not affected by CPU frequency scaling.
perf_measurement_t *measure_cycle_count;
// This counts context switches. Until Linux 2.6.34, these were all reported as user-space events, after that they are reported as happening in the kernel.
perf_measurement_t *measure_context_switches;
// This reports the CPU clock, a high-resolution per-CPU timer.
// See also: https://stackoverflow.com/questions/23965363/linux-perf-events-cpu-clock-and-task-clock-what-is-the-difference.
perf_measurement_t *measure_cpu_clock;
// This counts the number of branch misses branch misses. Retired branch instructions.  Prior to Linux 2.6.35, this used the wrong event on AMD processors
perf_measurement_t *measure_cpu_branches;

void print_results(measurement_t *measurements);

#endif
