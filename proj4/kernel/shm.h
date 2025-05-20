#ifndef _SHM_H_
#define _SHM_H_

#define SHMBASE (MAXVA - 16 * PGSIZE)  // Start of shared memory
#define SHM_MAX_PAGES 14

// Shared memory entry
struct shm_page {
  int key;
  void *pa;  // Physical address
  int refcount; // Record num of processes referencing shared page
};

struct shm_table_t{
  struct spinlock lock;
  struct shm_page pages[SHM_MAX_PAGES];
};

extern struct shm_table_t shm_table;

void shminit(void);

// Used to clean up shared memory on exit
void shmcleanup(pagetable_t pagetable);

#endif