#include <stdio.h>
#include <time.h>

#include "harness.h"

struct timespec hundred_milliseconds = {0, 100 * 1000000};

int perform_computation() {
  int result = 0;

  // Some costly computation
  for (int i = 0; i < 10000; i++)
    result = i + i * 2;

  // A bit of IO
  nanosleep(&hundred_milliseconds, NULL);

  return result;
}

int main(int argc, char **argv) {
  int result = 0;
  // Perform the test several times
  for (int i = 0; i < TEST_ITERATIONS; i++) {
    perf_start_measurement(all_measurements);
    // Carry out the computation
    result = perform_computation();
    perf_stop_measurement(all_measurements);
    perf_read_measurement(all_measurements, measurements + i, sizeof(measurement_t));
  }

  // Print the result, just as the original program would
  printf("Result: %d\n", result);
}

/**
* This is the original main function, to visualize what changes were made
* to support the measurements. The harness.c and harness.h files were not
* part of the original code and are not dependant on any code herein.
* It is general and may be used in multiple programs as it's self-isolated.
int original_main(int argc, char **argv) {
  int result = perform_computation();
  printf("Result: %d\n", result);
}
*/
