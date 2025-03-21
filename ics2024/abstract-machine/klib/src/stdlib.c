#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  // panic("Not implemented");
  // 在malloc()中维护一个上次分配内存位置的变量addr, 每次调用malloc()时, 就返回[addr, addr + size)这段空间. addr的初值设为heap.start, 表示从堆区开始分配. 你也可以参考microbench中的相关代码. 注意malloc()对返回的地址有一定的要求, 具体情况请RTFM.
  extern Area heap;
  static void *addr = NULL;
  if(addr == NULL) addr = heap.start;
  void *ret = addr;
  addr += size;
  return ret;
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
