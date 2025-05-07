#ifndef XV6TIMER_H
#define XV6TIMER_H

// Forward declare struct proc so we can reference it
struct proc;

typedef void (*xv6timer_callback_t)(struct xv6timer_t *);

struct xv6timer_t {
  int expiry;                      // Interval between activations
  uint next_tick;                 // Next tick to fire
  struct proc *proc;             // Associated process
  xv6timer_callback_t callback;  // Function to call
  struct xv6timer_t *next;       // For linked list of timers
};

struct xv6timer_t *alloc_timer(void);

void xv6timer_init(struct xv6timer_t *ptimer, struct proc *proc);
void xv6timer_forward(struct xv6timer_t *ptimer, int expiry);
void xv6timer_register_callback(struct xv6timer_t *ptimer, xv6timer_callback_t cb);
void xv6timer_callback(struct xv6timer_t *t);
void xv6_timers_tick(void); // called on every tick
void free_timer(struct xv6timer_t *t);

#endif
