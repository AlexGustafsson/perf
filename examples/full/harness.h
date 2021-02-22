#ifndef HARNESS_H
#define HARNESS_H

#include <perf/utilities.h>

#define TEST_ITERATIONS 100

uint64_t instruction_counts[TEST_ITERATIONS];
uint64_t cycle_counts[TEST_ITERATIONS];
uint64_t context_switches[TEST_ITERATIONS];
uint64_t cpu_clocks[TEST_ITERATIONS];

// Retired instructions. Be careful, these can be affected by various issues, most notably hardware interrupt counts.
perf_measurement_t *measure_instruction_count;
// Total cycles; not affected by CPU frequency scaling.
perf_measurement_t *measure_cycle_count;
// This counts context switches.  Until Linux 2.6.34, these were all reported as user-space events, after that they are reported as happening in the kernel.
perf_measurement_t *measure_context_switches;
// This reports the CPU clock, a high-resolution per-CPU timer.
// See also: https://stackoverflow.com/questions/23965363/linux-perf-events-cpu-clock-and-task-clock-what-is-the-difference.
perf_measurement_t *measure_cpu_clock;

#endif
