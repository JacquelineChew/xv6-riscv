// File-system system calls.

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

uint64
sys_calculate(void)
{
  struct proc *p = myproc();
  int x;
  int y;
  char* op;
  int result;
  uint64 op_addr, result_addr;

  argint(0, &x);
  argint(1, &y);
  argaddr(2, &op_addr);
  argaddr(3, &result_addr);

  if(copyin(p->pagetable, (char *)op, op_addr, sizeof(*op)) != 0)
    return -1;

  switch(op){
    case '+':
      result = x + y;
      break;
    case '-':
      result = x - y;
      break;
    case '*':
      result = x * y;
      break;
    case '/':
      result = x / y;
      break;
    default:
      return -1;
  }

  if(copyout(p->pagetable, result_addr, (char*)&result, sizeof(result)) != 0){
    return -1;
  }


  return 0;
}