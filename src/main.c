#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

#include <perf/utilities.h>

int main(int argc, char **argv) {
  if (perf_is_supported() != 0) {
    printf("Perf API not supported\n");
    exit(EXIT_FAILURE);
  }

  int event_paranoia = perf_get_event_paranoia();
  if (event_paranoia == -1) {
    printf("error: unable to get event paranoia\n");
    exit(EXIT_FAILURE);
  }
  if (event_paranoia & PERF_EVENT_PARANOIA_ALLOW_ALL)
    printf("Event paranoia: Allow All\n");
  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_CPU)
    printf("Event paranoia: Disallow CPU\n");
  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_FTRACE)
    printf("Event paranoia: Disallow ftrace\n");
  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_KERNEL)
    printf("Event paranoia: Disallow kernel\n");

  int has_sufficient_privilege = perf_has_sufficient_privilege(event_paranoia);
  if (has_sufficient_privilege == -1) {
    printf("error: unable to check for sufficient privilege\n");
    exit(EXIT_FAILURE);
  }
  printf("Has sufficient privilege: %d\n", has_sufficient_privilege);
  if (!has_sufficient_privilege) {
    printf("Unprivileged user\n");
    return EXIT_FAILURE;
  }

  perf_measurement_t *measure_instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
  if (measure_instruction_count == NULL) {
    printf("Failed to create measurement\n");
    exit(EXIT_FAILURE);
  }

  measure_instruction_count->attribute.exclude_hv = 1;
  measure_instruction_count->attribute.exclude_kernel = 1;

  if (perf_open_measurement(measure_instruction_count, getpid(), -1, -1, 0) == -1) {
    printf("Failed to open measurement\n");
    free((void*)measure_instruction_count);
    exit(EXIT_FAILURE);
  }

  perf_start_measurement(measure_instruction_count);

  printf("Measuring instruction count for this printf\n");

  perf_stop_measurement(measure_instruction_count);

  uint64_t instruction_count = 0;
  if (perf_read_measurement(measure_instruction_count, &instruction_count) == -1) {
    printf("Failed to read measurement\n");
    free((void*)measure_instruction_count);
    exit(EXIT_FAILURE);
  }

  printf("Got %lu instructions\n", instruction_count);

  free((void*)measure_instruction_count);

  // TODO: Enable to validate instruction count. Previously the count was consistently 58 for a printf statement.
  // If we add abstractions / further calls, perhaps it's useful to first measure the required cycles for doing nothing
  // and then remove the count. That way we can remove the cost of the library itself?

  return EXIT_SUCCESS;
}
