#include "kernel/types.h"
#include "user/user.h"

extern int uptime(void); 

int main() {
  printf("timertest: started\n");

  for (int i = 0; i < 5; i++) {
    int before = uptime();
    //sleep(50);
    int after = uptime();
    printf("timertest: alive (%d), slept for %d ticks\n", i, after - before);
  }

  exit(0);
}
