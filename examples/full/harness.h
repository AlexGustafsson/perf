#ifndef HARNESS_H
#define HARNESS_H

#include <perf/utilities.h>

#define TEST_ITERATIONS 100

uint64_t instruction_counts[TEST_ITERATIONS];

perf_measurement_t *measure_instruction_count;

#endif
