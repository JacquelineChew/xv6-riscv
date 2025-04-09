#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

int argint(int n, int *ip);
int argaddr(int n, uint64 *ip);

int sys_calculate(void) {
  int x, y;
  uint64 op_ptr, result_ptr;
  char op;
  int result;

  if (argint(0, &x) < 0 || argint(1, &y) < 0)
    return -1;

  if (argaddr(2, &op_ptr) < 0 || argaddr(3, &result_ptr) < 0)
    return -1;

  if (copyin(myproc()->pagetable, &op, op_ptr, sizeof(char)) < 0)
    return -1;

  switch (op) {
    case '+': result = x + y; break;
    case '-': result = x - y; break;
    case '*': result = x * y; break;
    case '/':
      if (y == 0)
        return -1;
      result = x / y; break;
    default:
      return -1;
  }

  if (copyout(myproc()->pagetable, result_ptr, (char *)&result, sizeof(int)) < 0)
    return -1;

  return 0;
}
