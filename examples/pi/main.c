#include <stdio.h>
#include <math.h>

#include "harness.h"
double PI_double = 3.14159265f / 4;
float PI_float = 3.1415929265f / 4;

double calculate_pi_double() {
  double pi = 0;

  for (int i = 1; fabs(pi - PI_double) > 0.0000001f; i++)
    pi += pow(-1, i + 1) / (2 * i -1);

  return pi * 4;
}

float calculate_pi_float() {
  float pi = 0;

  for (int i = 1; fabsf(pi - PI_float) > 0.0000001f; i++)
    pi += powf(-1, i + 1) / (2 * i -1);

  return pi * 4;
}

int main()
{
  double pi_double = 0;
  float pi_float = 0;
  measurement_t measurements_pi_double[TEST_ITERATIONS];
  measurement_t measurements_pi_float[TEST_ITERATIONS];

  // Perform the test several times
  for (int i = 0; i < TEST_ITERATIONS; i++)
  {
    perf_start_measurement(all_measurements);
    // Carry out the computation
    pi_double = calculate_pi_double();
    perf_stop_measurement(all_measurements);
    perf_read_measurement(all_measurements, measurements_pi_double + i, sizeof(measurement_t));

    perf_start_measurement(all_measurements);
    // Carry out the computation
    pi_float = calculate_pi_float();
    perf_stop_measurement(all_measurements);
    perf_read_measurement(all_measurements, measurements_pi_float + i, sizeof(measurement_t));
  }

  printf("Result calculating pi using double\n");
  print_results(measurements_pi_double);
  printf("pi: %f\n\n", pi_double);

  printf("Result calculating pi using float\n");
  print_results(measurements_pi_float);
  printf("pi : %f\n", pi_float);

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
