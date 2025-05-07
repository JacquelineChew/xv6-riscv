uint64
sys_setperiod(void)
{
  int ticks;  // Interval in ticks at which the process will wake up
  struct proc *p = myproc();

  argint(0, &ticks);
  if (ticks < 1)  // If period is invalid
    return -1;

  if (p->is_periodic)
    return 0; // Already registered

  if (num_periodic >= MAX_PERIODIC)
    return -1;

  // Allocate a timer from the global timer pool
  struct xv6timer_t *ptimer = alloc_timer();
  if (!ptimer)
    return -1;  // No available timers

  // Associates a periodic timer with the calling process
  xv6timer_init(ptimer, p);
  xv6timer_register_callback(ptimer, xv6timer_callback);
  xv6timer_forward(ptimer, ticks);

  // Set proc state
  p->ptimer = ptimer;
  p->period = ticks;
  p->is_periodic = 1;
  p->next_release = ticks + ticks;

  periodic_procs[num_periodic++] = p;

  return 0;
}
