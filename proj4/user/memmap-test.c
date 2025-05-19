#include "../../kernel/types.h"
#include "../../kernel/stat.h"
#include "../../user/user.h"
#include "../../kernel/riscv.h"

int main() { 
  
  printf("Before shmget is called\n");
  show_vm_areas();

  shmget(0, 4096);

  printf("\nAfter shmget is called\n");
  show_vm_areas();
  
  exit(0); 
}