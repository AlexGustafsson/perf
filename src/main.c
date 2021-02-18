#include <stdlib.h>
#include <stdio.h>

#include "perf.h"

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
  printf("Event paranoia: %d\n", event_paranoia);

  return EXIT_SUCCESS;
}
