struct spinlock sleeplock;

uint64 
sys_wait_until_next_period(void)
{
  struct proc *p = myproc();  // Return current struct proc
  acquire(&sleeplock);
  sleep(p, &sleeplock);
  return 0;
}