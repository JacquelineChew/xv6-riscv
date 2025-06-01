#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h" 

int main(int argc, char *argv[]) { 
  if (argc != 2) { 
    printf("Usage: getfilesize <filename>\n"); 
    exit(1); 
  } 
  int size; 
  if (getfilesize(&size, argv[1]) == 0) { 
    // printf("File %s size: %d bytes\n", argv[1], size); 
  } 
  else { 
    printf("Error: Could not get file size\n"); 
  } 
  exit(0); 
} 