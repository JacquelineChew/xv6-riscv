uint64
sys_pipe_rt(void)
{
  // uint64 fdarray; // user pointer to array of two integers
  // struct file *rf, *wf;
  // int fd0, fd1;
  // struct proc *p = myproc();

  // argaddr(0, &fdarray);
  // if(pipealloc(&rf, &wf) < 0)
  //   return -1;
  // fd0 = -1;
  // if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
  //   if(fd0 >= 0)
  //     p->ofile[fd0] = 0;
  //   fileclose(rf);
  //   fileclose(wf);
  //   return -1;
  // }

  return 0;
}