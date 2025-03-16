#include <common.h>
#include "syscall.h"
#include <sys/time.h>

;
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_open(const char *pathname, int flags, int mode);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_exit:{
      /* Log("Sys_Exit触发.\n"); */ 
      void* naive_uload(void *pcb, const char *filename);
      Log("Halt with code %d", a[1]);
      naive_uload(NULL, "/bin/menu");
      break;
    }
    case SYS_yield: /* Log("Sys_Yield触发\n"); */ c->GPRx = 0; break;
    case SYS_write:
    {
      /* Log("Sys_Write触发\n"); */
      c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
      break;
    }
    case SYS_brk: /* Log("Sys_Brk触发\n"); */ c->GPRx = 0; break;
    case SYS_open: /* Log("Sys_Open触发\n"); */ c->GPRx = fs_open((char*)a[1], a[2], a[3]); break;
    case SYS_read: /* Log("Sys_Read触发\n"); */ c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
    case SYS_close: /* Log("Sys_Close触发\n"); */ c->GPRx = 0; break;
    case SYS_lseek: /* Log("Sys_Lseek触发\n"); */ c->GPRx = fs_lseek(a[1], a[2], a[3]); break; 
    case SYS_gettimeofday:{
      /* Log("Sys_Gettimeofday触发\n"); */
      if(a[1] != 0){
        uint64_t us = io_read(AM_TIMER_UPTIME).us;
        struct timeval *tv = (struct timeval *)a[1];
        tv->tv_sec = us / 1000000; // get the second
        tv->tv_usec = us % 1000000; // get the time of the day
      }
      c->GPRx = 0;
      break;
    }
    case SYS_execve:{
      /* Log("Sys_Execve触发\n"); */ 
      void *naive_uload(void *pcb, const char *filename);
      int fd = fs_open((char*)a[1], 0, 0);
      if(fd < 0){
        c->GPRx = -8;
        break;
      }
      naive_uload(NULL, (char*)a[1]);
      c->GPRx = -13;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
