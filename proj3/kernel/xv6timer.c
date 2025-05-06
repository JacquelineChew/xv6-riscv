#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "xv6timer.h"

extern uint ticks;  // Defined in trap.c (Global tick count)

struct xv6timer_t mytimer;

// Initiates xv6timer with parameters, and associate it with a process
void xv6timer_init(struct xv6timer_t *ptimer, struct proc *proc) {
    ptimer->proc = proc;
    ptimer->expiry = 0;
    ptimer->next_tick = 0;
    ptimer->callback = 0;
}

// Reschedule the time and set the next expiry time
void xv6timer_forward(struct xv6timer_t *ptimer, int expiry) {
    ptimer->expiry = expiry;
    ptimer->next_tick = ticks + expiry;
    printf("xv6timer_forward: current tick %d, next tick set at %d\n", ticks, ptimer->next_tick);
}

// Register the timer with a callback which will be called when the time interrupt is triggered
void xv6timer_register_callback(struct xv6timer_t *ptimer, xv6timer_callback_t cb) {
    ptimer->callback = cb;
}

// Calls the registered callback when the interrupt is triggered
void xv6timer_interrupt(struct xv6timer_t *ptimer) {
    if (ptimer->callback && ticks >= ptimer->next_tick) {  // Check if ticks expired
        printf("xv6timer_interrupt at tick %d\n", ticks);
        ptimer->callback(ptimer);   // Call registered callback                  
    }
}

// Callback function wakes up process when interrupt is triggered
void xv6timer_callback(struct xv6timer_t *ptimer) {
    struct proc *p = ptimer->proc;
    wakeup(p);

    xv6timer_forward(ptimer, ptimer->expiry);   // Reschedule next tick
}

void setup_demo_timer(void) {
  xv6timer_init(&mytimer, 0);                     
  xv6timer_register_callback(&mytimer, xv6timer_callback);
  xv6timer_forward(&mytimer, 50);                  
}

