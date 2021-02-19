#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "harness.h"

int perform_computation() {
  int result = 0;

  for (int i = 0; i < 100; i++)
    result = i + i * 2;

  return result;
}

int main(int argc, char **argv) {
  int result = 0;
  for (int i = 0; i < TEST_ITERATIONS; i++) {
    perf_start_measurement(measure_instruction_count);
    result = perform_computation();
    perf_stop_measurement(measure_instruction_count);
    perf_read_measurement(measure_instruction_count, instruction_counts + i);
  }

  int sum = 0;
  for (int i = 0; i < TEST_ITERATIONS; i++)
    sum += instruction_counts[i];

  printf("Result: %d\n", result);
  printf("Measurements:\n");
  for (int i = 0; i < TEST_ITERATIONS; i++) {
    printf("%lu,\t", instruction_counts[i]);

    if (i > 0 && i % 10 == 0)
      printf("\n");
  }
  printf("\n");
  printf("Got an average of %u instructions\n", sum / TEST_ITERATIONS);
}
