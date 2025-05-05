#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "xv6timer.h"

struct xv6timer_t mytimer;

void xv6timer_init(struct xv6timer_t *ptimer, struct proc *proc) {
    ptimer->proc = proc;
    ptimer->expiry = 0;
    ptimer->next_tick = 0;
    ptimer->callback = 0;
}

void xv6timer_forward(struct xv6timer_t *ptimer, int expiry) {
    extern uint ticks;
    ptimer->expiry = expiry;
    ptimer->next_tick = ticks + expiry;
}


void xv6timer_register_callback(struct xv6timer_t *ptimer, xv6timer_callback_t cb) {
    ptimer->callback = cb;
}


void xv6timer_interrupt(struct xv6timer_t *ptimer) {
    extern uint ticks;

    if (ptimer->callback && ticks >= ptimer->next_tick) {
        ptimer->callback(ptimer);                     
        ptimer->next_tick = ticks + ptimer->expiry;   
    }
}

void my_callback(struct xv6timer_t *t) {
  printf("Tick callback! ticks=%d, next_tick=%d\n", ticks, t->next_tick);
}

void setup_demo_timer(void) {
  xv6timer_init(&mytimer, 0);                     
  xv6timer_register_callback(&mytimer, my_callback);
  xv6timer_forward(&mytimer, 50);                  
}

