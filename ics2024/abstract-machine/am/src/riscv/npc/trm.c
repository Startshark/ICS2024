#include <am.h>
#include <klib-macros.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
static const char mainargs[MAINARGS_MAX_LEN] = MAINARGS_PLACEHOLDER; // defined in CFLAGS

void putch(char ch) {
  char buf[2] = {ch, '\0'};
  ioe_write(AM_GPU_PUTCHAR, buf);
}

void halt(int code) {
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
