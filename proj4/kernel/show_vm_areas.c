// Prints the entire memory-mapped areas of a process
uint64
sys_show_vm_areas(void)
{
  // Syscall walks through entire page table of caller process;
  // Prints each pair of start and end addresses, and the size of the area (in num of pages)

  struct proc *p = myproc();
  pagetable_t pt = p->pagetable;

  printf("[Memory-mapped areas for process %d:]\n", p->pid);

  uint64 va_start = 0;
  uint64 va_end = 0;
  int in_region = 0; // Indicate being inside within a memory-mapped area
  int num_pages = 0;

  for(uint64 va = 0; va < MAXVA; va += PGSIZE){
    // Check if there's a phys. page for virtual addr.
    // pa = 0 if not mapped
    uint64 pa = walkaddr(pt, va);

    if (pa != 0){ // Virtual addr is mapped to a page
      if (!in_region){
        va_start = va;
        in_region = 1;
      }
    }
    else{
      if(in_region){
        va_end = va;
        num_pages = (va_end - va_start) / PGSIZE;
        printf("0x%lx - 0x%lx: %d\n", va_start, va_end, num_pages);
        in_region = 0;
      }
    }
  }

  // Handle the case where the last region reaches MAXVA
  if (in_region) {
    va_end = MAXVA;
    num_pages = (va_end - va_start) / PGSIZE;
    printf("0x%lx - 0x%lx: %d\n", va_start, va_end, num_pages);
  }

  return 0;
}