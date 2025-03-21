#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

struct Context {
  uintptr_t gpr[NR_REGS]; // 通用寄存器
  uintptr_t mcause;       // 异常原因
  uintptr_t mstatus;      // 机器状态寄存器
  uintptr_t mepc;         // 异常程序计数器
  void *pdir;             // 页目录指针  
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

#endif
