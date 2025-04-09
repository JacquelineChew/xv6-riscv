#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h" 

int main(int argc, char *argv[]) { 
  int res = 0; 
  char op = '+'; 
  int error_code = calculate(10, 20, &op, &res); 
  printf("%d, %d\n", res, error_code); 
  exit(0); 
} 