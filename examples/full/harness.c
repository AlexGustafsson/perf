#include <stdio.h>
#include <stdlib.h>

#include "harness.h"
#include "perf/utilities.h"

static int prepared_successfully = 0;

// Call prepare before executing main
void
prepare() __attribute__((constructor));
// Call cleanup before exiting
void cleanup() __attribute__((destructor));

void assert_support() {
  int major, minor, patch;
  int status = perf_get_kernel_version(&major, &minor, &patch);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Kernel version: %d.%d.%d\n", major, minor, patch);

  status = perf_is_supported();
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  } else if (status == 0) {
    fprintf(stderr, "error: perf not supported\n");
    exit(EXIT_FAILURE);
  }
}

void prepare_measurement(perf_measurement_t *measurement) {
  int status = perf_has_sufficient_privilege(measurement);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  } else if (status == 0) {
    fprintf(stderr, "error: unprivileged user\n");
    exit(EXIT_FAILURE);
  }

  status = perf_open_measurement(measurement, -1, 0);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  }
}

void prepare() {
  fprintf(stderr, "preparing harness\n");

  assert_support();

  measure_instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 0, -1);
  measure_instruction_count->attribute.exclude_kernel = 1;
  prepare_measurement(measure_instruction_count);

  measure_cycle_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, 0, -1);
  measure_cycle_count->attribute.exclude_kernel = 1;
  prepare_measurement(measure_cycle_count);

  measure_context_switches = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, 0, -1);
  prepare_measurement(measure_context_switches);

  measure_cpu_clock = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, 0, -1);

  prepared_successfully = 1;
}

void print_results() {
  int sum = 0;
  for (int i = 0; i < TEST_ITERATIONS; i++)
    sum += context_switches[i];

  printf("Measurements:\n");
  for (int i = 0; i < TEST_ITERATIONS; i++) {
    printf("%lu,\t", context_switches[i]);

    if (i > 0 && i % 10 == 0)
      printf("\n");
  }
  printf("\n");
  printf("Got an average of %u context switches\n", sum / TEST_ITERATIONS);
}

void cleanup() {
  if (prepared_successfully)
    print_results();

  fprintf(stderr, "cleaning up harness\n");
  if (measure_instruction_count != NULL) free((void*)measure_instruction_count);
  if (measure_cycle_count != NULL) free((void*)measure_cycle_count);
  if (measure_context_switches != NULL) free((void*)measure_context_switches);
}
