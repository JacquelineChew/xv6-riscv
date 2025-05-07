uint64 
sys_wait_until_next_period(void)
{
  struct proc *p = myproc();

  acquire(&p->lock);
  p->state = SLEEPING;
  sched();  // yield CPU
  release(&p->lock);

  return 0;
}
