#include <inttypes.h>
#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <perf/utilities.h>

typedef struct {
  uint64_t nr;
  struct {
    uint64_t value;
    uint64_t id;
  } values[1];
} measurement_t;

int main(int argc, char **argv) {
  // Create a measurement using hardware (CPU) registers. Measure the number of instructions amassed.
  perf_measurement_t *measure_instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 0, -1);

  // Ensure that the caller has sufficient privilege for performing the measurement
  int has_sufficient_privilege = perf_has_sufficient_privilege(measure_instruction_count);
  if (has_sufficient_privilege != 1) {
    fprintf(stderr, "Insufficient privilege\n");
    return EXIT_FAILURE;
  }

  // Ensure that the event is supported
  int is_supported = perf_event_is_supported(measure_instruction_count);
  if (is_supported != 1) {
    fprintf(stderr, "Measuring hardware instructions is not supported\n");
    return EXIT_FAILURE;
  }

  // Open the measurement (register the measurement, but don't start measuring)
  perf_open_measurement(measure_instruction_count, -1, 0);

  // Reset the counter and start measuring
  perf_start_measurement(measure_instruction_count);

  // Perform a computation
  printf("Measuring instruction count for this printf\n");

  // Stop the counter
  perf_stop_measurement(measure_instruction_count);

  // Read the number of instructions from the counter
  measurement_t measurement;
  perf_read_measurement(measure_instruction_count, &measurement, sizeof(measurement_t));

  // Print the instruction count
  printf("%" PRIu64 "\n", measurement.nr);
  printf("%" PRIu64 "\n", measurement.values[0].id);
  printf("%" PRIu64 "\n", measurement.values[0].value);

  // Always close a created measurement
  perf_close_measurement(measure_instruction_count);

  // Always free any allocated measurement
  free((void *)measure_instruction_count);

  return EXIT_SUCCESS;
}
