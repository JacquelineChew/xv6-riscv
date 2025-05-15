#include "../../kernel/types.h"
#include "../../kernel/stat.h"
#include "../../user/user.h"

int main() { 
  
  show_vm_areas();

  shmget(1,1);
  
  exit(0); 
}