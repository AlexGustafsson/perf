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
  perf_start_measurement(measure_instruction_count);

  int result = perform_computation();

  perf_stop_measurement(measure_instruction_count);

  uint64_t instruction_count = 0;
  perf_read_measurement(measure_instruction_count, &instruction_count);
  printf("%lu instructions to calculate result: %d\n", instruction_count, result);
}
