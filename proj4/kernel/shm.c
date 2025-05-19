#include "../../kernel/types.h"
#include "../../kernel/param.h"
#include "../../kernel/memlayout.h"
#include "../../kernel/riscv.h"
#include "../../kernel/spinlock.h"
#include "../../kernel/proc.h"
#include "../../kernel/defs.h"
#include "shm.h"

struct shm_table_t shm_table;

void
shminit(void) {
  initlock(&shm_table.lock, "shm_table");
  for (int i = 0; i < SHM_MAX_PAGES; i++) {
    shm_table.pages[i].key = -1;
    shm_table.pages[i].pa = 0;
    shm_table.pages[i].refcount = 0;
  }
}
