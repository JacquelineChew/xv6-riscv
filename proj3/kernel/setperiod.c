uint64 
sys_setperiod(void)
{
  int ticks;  // Interval in ticks at which the process will wake up
  argint(0, &ticks);
  if (ticks < 1) return -1;   // If period is invalid

  // Associates a periodic timer with the calling process
  xv6timer_init(&mytimer, myproc());
  xv6timer_register_callback(&mytimer, xv6timer_callback);
  xv6timer_forward(&mytimer, ticks);

  return 0;
}