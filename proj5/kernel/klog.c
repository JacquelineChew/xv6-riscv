
#include "../../kernel/types.h"
#include "../../kernel/param.h"
#include "../../kernel/memlayout.h"
#include "../../kernel/riscv.h"
#include "../../kernel/spinlock.h"
#include "../../kernel/sleeplock.h"
#include "../../kernel/proc.h"
#include "../../kernel/defs.h"
#include "../../kernel/fs.h"
#include "../../kernel/file.h"

#define KLOG_BUF_SIZE 1024

static struct {
  char buf[KLOG_BUF_SIZE];
  uint rpos;  // read position
  uint wpos;  // write position

  struct spinlock lock;
} klog;

// Send one character to the klog buffer
static void
klog_putc(char c)
{
  klog.buf[klog.wpos++ % KLOG_BUF_SIZE] = c;
}

static void
print_uint_to_klogbuf(uint num)
{
  // Temporary buffer to store digits in reverse order
  char temp[10];
  int i = 0;

  if (num == 0) {
    klog_putc('0');
    return;
  }

  while (num > 0 && i < sizeof(temp)) {
    temp[i++] = '0' + (num % 10);
    num /= 10;
  }

  // Output digits in correct order
  while (i > 0) {
    klog_putc(temp[--i]);
  }
}

// Write message logs to klog device
void
_printk(const char *s)
{
  acquire(&klog.lock);

  // Print timestamp prefix
  klog_putc('[');
  print_uint_to_klogbuf(ticks);  // no snprintf
  klog_putc(']');
  klog_putc(' ');

  // Print actual message
  for (int i = 0; s[i] != '\0'; i++) {
    klog_putc(s[i]);
  }

  klog_putc('\n');

  release(&klog.lock);
}

// Read from klog device
int
klogread(int user_dst, uint64 dst, int n)
{
  acquire(&klog.lock);
  int i;
  for (i = 0; i < n; i++) {
    if (klog.rpos == klog.wpos)
      break;
    char c = klog.buf[klog.rpos++ % KLOG_BUF_SIZE];
    if (copyout(myproc()->pagetable, dst + i, &c, 1) < 0)
      break;
  }
  release(&klog.lock);
  return i;
}


void
kloginit(void)
{
  initlock(&klog.lock, "klog");

  //uartinit();

  klog.rpos = 0;
  klog.wpos = 0;

  // connect read and write system calls
  // to klogread and klogwrite.
  devsw[KLOG].read = klogread;
  // devsw[KLOG].write = _printk;
}