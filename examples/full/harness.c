#include <stdio.h>
#include <stdlib.h>

#include "harness.h"
#include "perf/utilities.h"

// Call prepare before executing main
void prepare() __attribute__((constructor));
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

  prepare_measurement(measure_instruction_count);
}

void cleanup() {
  fprintf(stderr, "cleaning up harness\n");
  if (measure_instruction_count != NULL) free((void*)measure_instruction_count);
}
