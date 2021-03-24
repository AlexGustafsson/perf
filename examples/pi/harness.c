#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "harness.h"
#include "perf/utilities.h"

static int prepared_successfully = 0;

// Call prepare before executing main
void prepare() __attribute__((constructor));
// Call cleanup before exiting
void cleanup() __attribute__((destructor));

void assert_support() {
  // Print the kernel version
  int major, minor, patch;
  int status = perf_get_kernel_version(&major, &minor, &patch);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Kernel version: %d.%d.%d\n", major, minor, patch);

  // Exit if the API is unsupported
  status = perf_is_supported();
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  } else if (status == 0) {
    fprintf(stderr, "error: perf not supported\n");
    exit(EXIT_FAILURE);
  }
}

void prepare_measurement(const char *description, perf_measurement_t *measurement, perf_measurement_t *parent_measurement) {
  int status = perf_has_sufficient_privilege(measurement);
  if (status < 0) {
    perf_print_error(status);
    exit(EXIT_FAILURE);
  } else if (status == 0) {
    fprintf(stderr, "error: unprivileged user\n");
    exit(EXIT_FAILURE);
  }

  int support_status = perf_event_is_supported(measurement);
  if (support_status < 0) {
    perf_print_error(support_status);
    exit(EXIT_FAILURE);
  } else if (support_status == 0) {
    fprintf(stderr, "warning: %s not supported\n", description);
    return;
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

  // Fail if the perf API is unsupported
  assert_support();

  // Create a dummy measurement (measures nothing) to act as a group leader
  all_measurements = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_DUMMY, 0, -1);
  prepare_measurement("software dummy counter", all_measurements, NULL);

  // Measure the number of retired instructions
  measure_instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 0, -1);
  measure_instruction_count->attribute.exclude_kernel = 1;
  prepare_measurement("hardware instruction counter", measure_instruction_count, all_measurements);

  // Measure the number of CPU cycles (at least on Intel CPUs, see https://perf.wiki.kernel.org/index.php/Tutorial#Default_event:_cycle_counting)
  measure_cycle_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, 0, -1);
  measure_cycle_count->attribute.exclude_kernel = 1;
  prepare_measurement("hardware cycles counter", measure_cycle_count, all_measurements);

  // Measure the number of context switches
  measure_context_switches = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, 0, -1);
  prepare_measurement("software context switches counter", measure_context_switches, all_measurements);

  // Measure the CPU clock related to the task (see https://stackoverflow.com/questions/23965363/linux-perf-events-cpu-clock-and-task-clock-what-is-the-difference)
  measure_cpu_clock = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, 0, -1);
  prepare_measurement("software task clock", measure_cpu_clock, all_measurements);

  // Measure CPU branches
  measure_cpu_branches = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES, 0, -1);
  prepare_measurement("cpu branches counter", measure_cpu_branches, all_measurements);

  // Mark the preparation stage as successfuly
  prepared_successfully = 1;
}

void print_results(measurement_t *measurements) {
  if (!prepared_successfully)
    return;

  printf("     instructions          cycles  context switches            clock     cpu branches\n");
  for (int i = 0; i < TEST_ITERATIONS; i++) {
    uint64_t values[6] = {0};
    perf_measurement_t *taken_measurements[] = {all_measurements, measure_instruction_count, measure_cycle_count, measure_context_switches, measure_cpu_clock, measure_cpu_branches};

    for (uint64_t j = 0; j < measurements[i].recorded_values; j++) {
      for (int k = 0; k < 6; k++) {
        if (measurements[i].values[j].id == taken_measurements[k]->id) {
          values[k] = measurements[i].values[j].value;
          break;
        }
      }
    }

    // Ignore the results from the dummy counter
    printf("%17" PRIu64 "%17" PRIu64 "%17" PRIu64 "%17" PRIu64 "%17" PRIu64 "\n", values[1], values[2], values[3], values[4], values[5]);
  }
}

void cleanup() {
  fprintf(stderr, "cleaning up harness\n");
  if (all_measurements != NULL) {
    perf_close_measurement(all_measurements);
    free((void *)all_measurements);
  }

  if (measure_instruction_count != NULL) {
    perf_close_measurement(measure_instruction_count);
    free((void *)measure_instruction_count);
  }

  if (measure_cycle_count != NULL) {
    perf_close_measurement(measure_cycle_count);
    free((void *)measure_cycle_count);
  }

  if (measure_context_switches != NULL) {
    perf_close_measurement(measure_context_switches);
    free((void *)measure_context_switches);
  }

  if (measure_cpu_clock != NULL) {
    perf_close_measurement(measure_cpu_clock);
    free((void *)measure_cpu_clock);
  }
}
