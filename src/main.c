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
  if (event_paranoia & PERF_EVENT_PARANOIA_ALLOW_ALL) printf("Event paranoia: Allow All\n");
  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_CPU) printf("Event paranoia: Disallow CPU\n");
  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_FTRACE) printf("Event paranoia: Disallow ftrace\n");
  if (event_paranoia & PERF_EVENT_PARANOIA_DISALLOW_KERNEL) printf("Event paranoia: Disallow kernel\n");

  int has_sufficient_privilege = perf_has_sufficient_privilege(event_paranoia);
  if (has_sufficient_privilege == -1) {
    printf("error: unable to check for sufficient privilege\n");
    exit(EXIT_FAILURE);
  }
  printf("Has sufficient privilege: %d\n", has_sufficient_privilege);

  return EXIT_SUCCESS;
}
