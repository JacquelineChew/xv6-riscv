// Enable shared memory between multiple processes by allocating pages to caller process
// Returns the virtual addr defined
uint64
sys_shmget(void)
{
  int key; // Page identifier in physical memory
  uint64 size;  // Num of bytes for shared region

  argint(0, &key);
  argaddr(1, &size);

  if (key < 0 || size <= 0 || size >= 16 * PGSIZE) return -1;

  struct proc *p = myproc();
  uint64 va = SHMBASE + (key % SHM_MAX_PAGES) * PGSIZE;

  acquire(&shm_table.lock);

  // Search for existing entry
  struct shm_page *e = 0;
  for (int i = 0; i < SHM_MAX_PAGES; i++) {
    if (shm_table.pages[i].key == key) {
      e = &shm_table.pages[i];
      break;
    }
  }

  if (e) {
    mappages(p->pagetable, va, PGSIZE, (uint64)e->pa, PTE_R | PTE_W | PTE_U);
    e->refcount++;
  } else {
    for (int i = 0; i < SHM_MAX_PAGES; i++) {
      if (shm_table.pages[i].key == -1) {
        e = &shm_table.pages[i];
        e->key = key;
        e->pa = kalloc();
        if (!e->pa) {
          release(&shm_table.lock);
          return -1;
        }
        e->refcount = 1;
        memset((void *)e->pa, 0, size);
        mappages(p->pagetable, va, PGSIZE, (uint64)e->pa, PTE_R | PTE_W | PTE_U);
        break;
      }
    }
  }

  release(&shm_table.lock);
  return va;
}