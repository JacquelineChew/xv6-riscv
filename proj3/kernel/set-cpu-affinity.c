uint64
sys_set_cpu_affinity(void)
{
  int mask;
  argint(0, &mask);

  struct proc *p = myproc();  // Return current struct proc
  p->cpu_mask = mask;

  return 0;
}