#include "../../kernel/types.h"
#include "../../kernel/stat.h"
#include "../../user/user.h"
#include "../../kernel/riscv.h"

int main() { 

  printf("MAXVA: %lx\n", MAXVA);
  
  show_vm_areas();

  shmget(1,1);
  
  exit(0); 
}