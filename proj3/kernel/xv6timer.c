#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "xv6timer.h"

extern uint ticks;  // from trap.c
#define MAX_PERIODIC 4
static struct xv6timer_t timers[MAX_PERIODIC];
static int timer_used[MAX_PERIODIC] = {0}; // 0 = free, 1 = used


// Singly linked list of timers
static struct {
  struct spinlock lock;
  struct xv6timer_t *head;
} timerlist;

// Initialize a timer and associate it with a process
void xv6timer_init(struct xv6timer_t *ptimer, struct proc *proc) {
  ptimer->proc = proc;
  ptimer->expiry = 0;
  ptimer->next_tick = 0;
  ptimer->callback = 0;
  ptimer->next = 0;

  acquire(&timerlist.lock);
  ptimer->next = timerlist.head;
  timerlist.head = ptimer;
  release(&timerlist.lock);
}

// Schedule the timer for a future tick
void xv6timer_forward(struct xv6timer_t *ptimer, int expiry) {
  ptimer->expiry = expiry;
  ptimer->next_tick = ticks + expiry;
}

// Register a callback for this timer
void xv6timer_register_callback(struct xv6timer_t *ptimer, xv6timer_callback_t cb) {
  ptimer->callback = cb;
}

// Called every tick to check timers
void xv6_timers_tick(void) {
  acquire(&timerlist.lock);
  for (struct xv6timer_t *t = timerlist.head; t != 0; t = t->next) {
    if (t->callback && ticks >= t->next_tick) {
      t->callback(t);
    }
  }
  release(&timerlist.lock);
}

struct xv6timer_t *alloc_timer(void) {
  for (int i = 0; i < MAX_PERIODIC; i++) {
    if (timer_used[i] == 0) {
      timer_used[i] = 1;
      return &timers[i];
    }
  }
  return 0; // No available timer
}

void free_timer(struct xv6timer_t *t) {
  for (int i = 0; i < MAX_PERIODIC; i++) {
    if (&timers[i] == t) {
      timer_used[i] = 0;
      break;
    }
  }
}

// Default callback: wake up the sleeping process and reschedule
void xv6timer_callback(struct xv6timer_t *ptimer) {
  struct proc *p = ptimer->proc;
  acquire(&p->lock);
  if (p->state == SLEEPING)
    p->state = RUNNABLE;
  release(&p->lock);

  xv6timer_forward(ptimer, ptimer->expiry); // Reschedule
}


