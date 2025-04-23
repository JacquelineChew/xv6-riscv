#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"


#define PIPESIZE 512
#define MAX_TASKS 64

typedef struct task_t {
  int priority;
  int x;
  int y;
  char op; // Supports "+", "-", "*", "/"
  int result;
  int error;
} task_t;

struct pipe {
  struct spinlock lock;
  char data[PIPESIZE];
  task_t taskbuf[MAX_TASKS];
  uint nread;     // number of bytes read
  uint nwrite;    // number of bytes written
  int readopen;   // read fd is still open
  int writeopen;  // write fd is still open
  int is_priority; // 1 if pipe_rt, 0 if normal
};

#include "proj2/kernel/pipe_rt_fcns.c"

int
pipealloc(struct file **f0, struct file **f1)
{
  struct pipe *pi;

  pi = 0;
  *f0 = *f1 = 0;
  if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
    goto bad;
  if((pi = (struct pipe*)kalloc()) == 0)
    goto bad;
  pi->readopen = 1;
  pi->writeopen = 1;
  pi->nwrite = 0;
  pi->nread = 0;
  initlock(&pi->lock, "pipe");
  (*f0)->type = FD_PIPE;
  (*f0)->readable = 1;
  (*f0)->writable = 0;
  (*f0)->pipe = pi;
  (*f1)->type = FD_PIPE;
  (*f1)->readable = 0;
  (*f1)->writable = 1;
  (*f1)->pipe = pi;
  return 0;

 bad:
  if(pi)
    kfree((char*)pi);
  if(*f0)
    fileclose(*f0);
  if(*f1)
    fileclose(*f1);
  return -1;
}

void
pipeclose(struct pipe *pi, int writable)
{
  acquire(&pi->lock);
  if(writable){
    pi->writeopen = 0;
    wakeup(&pi->nread);
  } else {
    pi->readopen = 0;
    wakeup(&pi->nwrite);
  }
  if(pi->readopen == 0 && pi->writeopen == 0){
    release(&pi->lock);
    kfree((char*)pi);
  } else
    release(&pi->lock);
}

int
pipewrite(struct pipe *pi, uint64 addr, int n)
{
  struct proc *pr = myproc();
  task_t task;

  if(n != sizeof(task_t)) return -1;

  acquire(&pi->lock);

  if(pi->writeopen == 0 || killed(pr)){
    release(&pi->lock);
    return -1;
  }

  if(copyin(pr->pagetable, (char*)&task, addr, sizeof(task_t)) < 0){
    release(&pi->lock);
    return -1;
  }
  printf("[kernel] received task: priority=%d x=%d y=%d op=%d result=%d error=%d\n",
    task.priority, task.x, task.y, task.op, task.result, task.error);

  if(pi->nwrite >= PIPESIZE){
    release(&pi->lock);
    return -1;    // Queue is full
  }

  // Insert with priority if priority pipe (higher priority first)
  int idx = pi->nwrite;
  while(idx > pi->nread && pi->is_priority &&
        pi->taskbuf[idx - 1].priority < task.priority){

    pi->taskbuf[idx] = pi->taskbuf[idx - 1];
    idx--;
  }
  pi->taskbuf[idx] = task;
  pi->nwrite++;

  wakeup(&pi->nread);
  release(&pi->lock);
  return sizeof(task_t);

  // int i = 0;
  // struct proc *pr = myproc();

  // acquire(&pi->lock);
  // while(i < n){
  //   if(pi->readopen == 0 || killed(pr)){
  //     release(&pi->lock);
  //     return -1;
  //   }
  //   if(pi->nwrite == pi->nread + PIPESIZE){ //DOC: pipewrite-full
  //     wakeup(&pi->nread);
  //     sleep(&pi->nwrite, &pi->lock);
  //   } else {
  //     char ch;
  //     if(copyin(pr->pagetable, &ch, addr + i, 1) == -1)
  //       break;
  //     pi->data[pi->nwrite++ % PIPESIZE] = ch;
  //     i++;
  //   }
  // }
  // wakeup(&pi->nread);
  // release(&pi->lock);

  // return i;
}

int
piperead(struct pipe *pi, uint64 addr, int n)
{
  struct proc *pr = myproc();
  int ret = -1;

  if(n != sizeof(task_t)) return -1;

  acquire(&pi->lock);

  while(pi->nread == pi->nwrite && pi->writeopen){  //DOC: pipe-empty
    if(killed(pr)){
      release(&pi->lock);
      return -1;
    }
    sleep(&pi->nread, &pi->lock); //DOC: piperead-sleep
  }

  if(pi->nread < pi->nwrite){
    if(copyout(pr->pagetable, addr, (char*)&pi->taskbuf[pi->nread], sizeof(task_t)) == 0){
      pi->nread++;
      ret = sizeof(task_t);
    }
  }

  wakeup(&pi->nwrite);   //DOC: piperead-wakeup
  release(&pi->lock);
  return ret;

  // int i;
  // struct proc *pr = myproc();
  // char ch;

  // acquire(&pi->lock);
  // while(pi->nread == pi->nwrite && pi->writeopen){  //DOC: pipe-empty
  //   if(killed(pr)){
  //     release(&pi->lock);
  //     return -1;
  //   }
  //   sleep(&pi->nread, &pi->lock); //DOC: piperead-sleep
  // }
  // for(i = 0; i < n; i++){  //DOC: piperead-copy
  //   if(pi->nread == pi->nwrite)
  //     break;
  //   ch = pi->data[pi->nread++ % PIPESIZE];
  //   if(copyout(pr->pagetable, addr + i, &ch, 1) == -1)
  //     break;
  // }
  // wakeup(&pi->nwrite);  //DOC: piperead-wakeup
  // release(&pi->lock);
  // return i;
}
