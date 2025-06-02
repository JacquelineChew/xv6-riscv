extern void _printk(const char *s);

// Syscall returns the size of a file
uint64
sys_getfilesize(void)
{
  struct proc *p = myproc();
  char filepath[MAXPATH];
  uint size;
  uint64 size_addr;
  argaddr(0, &size_addr);
  if (argstr(1, filepath, MAXPATH) < 0){
    return -1;
  }

  _printk("Calling getfilesize\n"); //_printk defined in klog.c

  struct inode *ip;

  begin_op(); // Mark start of FS system call
  if((ip = namei(filepath)) == 0){  // namei returns inode of file 
    end_op(); // Mark end of FS system call
    printf("File not found.\n");
    return -1;
  }
  

  ilock(ip);
  size = ip->size;  // Get size from inode struct
  iunlock(ip);
  end_op();

  if(copyout(p->pagetable, size_addr, (char*)&size, sizeof(size)) < 0){
    return -1;
  }

  return 0;
}