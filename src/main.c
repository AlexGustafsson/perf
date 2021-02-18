#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <perf/perf.h>
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
  if (!has_sufficient_privilege)
    return EXIT_FAILURE;

  pid_t pid = getpid();
  struct perf_event_attr attr;
  memset(&attr, 0, sizeof(attr));
  attr.type = PERF_TYPE_HARDWARE;
  attr.size = sizeof(attr);
  attr.config = PERF_COUNT_HW_INSTRUCTIONS;
  attr.disabled = 1;
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;

  int fd = perf_event_open(&attr, pid, -1, -1, 0);
  if (fd == -1) {
    fprintf(stderr, "Error opening leader %llx\n", attr.config);
    exit(EXIT_FAILURE);
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

  printf("Measuring instruction count for this printf\n");

  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  long long instructions_count = 0;
  read(fd, &instructions_count, sizeof(instructions_count));

  printf("Used %lld instructions\n", instructions_count);

  close(fd);

  return EXIT_SUCCESS;
}
