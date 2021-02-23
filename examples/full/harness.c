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

void prepare_measurement(perf_measurement_t *measurement, perf_measurement_t *parent_measurement) {
  int status = perf_has_sufficient_privilege(measurement);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  } else if (status == 0) {
    fprintf(stderr, "error: unprivileged user\n");
    exit(EXIT_FAILURE);
  }

  int group = parent_measurement == NULL ? -1 : parent_measurement->file_descriptor;

  status = perf_open_measurement(measurement, group, 0);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  }
}

void prepare() {
  fprintf(stderr, "preparing harness\n");

  assert_support();

  all_measurements = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_DUMMY, 0, -1);
  prepare_measurement(all_measurements, NULL);

  measure_instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 0, -1);
  measure_instruction_count->attribute.exclude_kernel = 1;
  prepare_measurement(measure_instruction_count, all_measurements);

  measure_cycle_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, 0, -1);
  measure_cycle_count->attribute.exclude_kernel = 1;
  prepare_measurement(measure_cycle_count, all_measurements);

  measure_context_switches = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, 0, -1);
  prepare_measurement(measure_context_switches, all_measurements);

  measure_cpu_clock = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, 0, -1);
  prepare_measurement(measure_cpu_clock, all_measurements);

  prepared_successfully = 1;
}

void print_results() {
  printf("    instructions           cycles  context switches           clock\n");
  for (int i = 0; i < TEST_ITERATIONS; i++) {
    uint64_t values[5] = {0};
    perf_measurement_t *taken_measurements[] = {all_measurements, measure_instruction_count, measure_cycle_count, measure_context_switches, measure_cpu_clock};

    for (uint64_t j = 0; j < measurements[i].nr; j++) {
      for (int k = 0; k < 5; k++) {
        if (measurements[i].values[j].id == taken_measurements[k]->id) {
          values[k] = measurements[i].values[j].value;
          break;
        }
      }
    }

    printf("%16lu %16lu %16lu %16lu\n", values[1], values[2], values[3], values[4]);
  }
}

void cleanup() {
  if (prepared_successfully)
    print_results();

  fprintf(stderr, "cleaning up harness\n");
  if (all_measurements != NULL) free((void*)all_measurements);
  if (measure_instruction_count != NULL) free((void*)measure_instruction_count);
  if (measure_cycle_count != NULL) free((void*)measure_cycle_count);
  if (measure_context_switches != NULL) free((void*)measure_context_switches);
  if (measure_cpu_clock != NULL) free((void*)measure_cpu_clock);
}
