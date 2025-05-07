#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void periodic_task(int period) {
  if (setperiod(period) < 0) {
    printf("PID %d: setperiod(%d) failed (possibly due to limit)\n", getpid(), period);
    exit(1);
  }

  while (1) {
    printf("PID %d: running (period %d)\n", getpid(), period);
    wait_until_next_period();  // Sleep until next release
  }
}

void non_periodic_task() {
  while (1) {
    printf("PID %d: non-periodic task running\n", getpid());
    sleep(20);  
  }
}

int main() {
  // int cpuid = 1; 
  // set_cpu_affinity(1 << cpuid);   /* pin the process to cpu 1 */
  // printf("Running on CPU %d\n", cpuid);

  int periods[] = {5, 10, 15, 20};  // 4 valid periodic periods

  // Create 5 processes (supports up to 4 periodic tasks)
  int i;
  for (i = 0; i < 5; i++) {
    if (fork() == 0) {
      if (i < 4) periodic_task(periods[i]);
      else{
        // Attempt to start a 5th periodic task (should fail)
        printf("PID %d: attempting to become 5th periodic task\n", getpid());
        if (setperiod(25) < 0) {
          printf("PID %d: correctly rejected as 5th periodic task\n", getpid());
          exit(0);
        } else {
          printf("PID %d: ERROR — should not have been accepted as periodic!\n", getpid());
          exit(1);
        }
        non_periodic_task();
      }
    }
  }

  // // Attempt to start a 5th periodic task (should fail)
  // if (i == 4 && fork() == 0) {
  //   printf("PID %d: attempting to become 5th periodic task\n", getpid());
  //   if (setperiod(25) < 0) {
  //     printf("PID %d: correctly rejected as 5th periodic task\n", getpid());
  //     exit(0);
  //   } else {
  //     printf("PID %d: ERROR — should not have been accepted as periodic!\n", getpid());
  //     exit(1);
  //   }
  // }

  // // Spawn a non-periodic background task
  // if (i == 4 && fork() == 0) {
  //   non_periodic_task();
  // }

  // Parent waits for children (won’t return unless child exits)
  // while (wait(0) >= 0);
  wait(0);

  exit(0);
}
