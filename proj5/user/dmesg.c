#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h" 

int main() { 
  int fd = open("klog", O_RDONLY); 
  if (fd < 0) { 
    printf("dmesg: cannot open klog\n"); 
    exit(1); 
  }

  char buf[256]; 
  int n; 

  while ((n = read(fd, buf, sizeof(buf))) > 0) { 
    write(1, buf, n); // write to stdout 
  } 
  
  close(fd); 
  exit(0);
}