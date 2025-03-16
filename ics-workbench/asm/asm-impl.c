#include "asm.h"
#include <string.h>
// #include <stdio.h>

int64_t asm_add(int64_t a, int64_t b) {
  asm("add %1, %0" : "+r"(a) : "r"(b));
  return a;
}

int asm_popcnt(uint64_t x) {
  uint64_t s;
  // int s = 0; 
  // for (int i = 0; i < 64; i++) {
  //   if ((x >> i) & 1) s++;
  // }
  // return s;
  asm("popcnt %1, %0" : "=r"(s) : "r"(x));
  return (int)s;
}

void *asm_memcpy(void *dest, const void *src, size_t n) {
  // return memcpy(dest, src, n);
  // asm ("rep movsb\n" : : "D"(dest), "S"(src), "c"(n));
  asm("rep movsb" : : "D"(dest), "S"(src), "c"(n));
  return dest;
}

int asm_setjmp(asm_jmp_buf env) {
  // return setjmp(env);
  asm volatile (
    "movq %%rbx, 8(%0)\n"
    "movq %%r12, 32(%0)\n"
    "movq %%r13, 40(%0)\n"
    "movq %%r14, 48(%0)\n"
    "movq %%r15, 56(%0)\n"

    "movq (%%rsp), %%rax\n" // rsp
    "movq %%rax, 16(%0)\n"
    
    "movq %%rbp, 0(%0)\n"
    
    "leaq 8(%%rsp), %%rax\n" // rip
    "movq %%rax, 24(%0)\n" // 保存返回地址到 env 中
    "xorq %%rax, %%rax\n"
    : /* 无输出 */
    : "r"(env)
    :
  );
  return 0;
}

// typedef struct {
// uint64_t rbx;
// uint64_t rbp;
// uint64_t r12;
// uint64_t r13;
// uint64_t r14;
// uint64_t r15;
// uint64_t rip;
// uint64_t rsp;
// } asm_jmp_buf[1];

void asm_longjmp(asm_jmp_buf env, int val) {
  // longjmp(env, val);
  asm volatile (
    "movl %1, %%eax\n"
    "movq 0(%0), %%rbp\n"
    "movq 8(%0), %%rbx\n"
    "movq 32(%0), %%r12\n"
    "movq 40(%0), %%r13\n"
    "movq 48(%0), %%r14\n"
    "movq 56(%0), %%r15\n"

    "movq 16(%0), %%rdx\n" // 恢复栈指针rsp
    
    "testl %%eax, %%eax\n" // 看看 val 是否为 0
    "jnz 1f\n"
    "inc %%eax\n"
    "1: movq 24(%0), %%rsp\n" // 恢复返回地址到 rdx
    "jmp *%%rdx\n" // 跳转, 好像是不能直接改动pc寄存器的值
    :
    : "r"(env), "r"(val)
    :
  );
}