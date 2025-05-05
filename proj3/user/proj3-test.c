#include "../../kernel/types.h"
#include "../../kernel/stat.h"
#include "../../user/user.h"

int main() { 
  
  int cpuid = 1; 
  set_cpu_affinity(1 << cpuid);   /* pin the process to cpu 0 */

  while (1) {
    printf("Running on CPU %d\n", cpuid);
    sleep(100);
  }

  setperiod(1);
  wait_until_next_period();
  
  exit(0); 
} 
