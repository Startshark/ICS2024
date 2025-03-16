#ifndef ITRACE_H
#define ITRACE_H

#include <common.h>
#include <elf.h>
#include <../../../include/device/map.h>

// 常量定义
#define MAX_IRINGBUF 16
#define ftrace_write log_write
#define dtrace_write log_write

typedef struct {
  word_t pc;
  uint32_t inst;
} ItraceNode;

typedef struct {
  char name[32];
  paddr_t addr;
  unsigned char info;
  Elf64_Xword size;
} SymEntry;

typedef struct tail_rec_node {
  paddr_t pc;
  paddr_t depend;
  struct tail_rec_node *next;
} TailRecNode;

// 外部变量声明
extern ItraceNode iringbuf[MAX_IRINGBUF];
extern int p_cur;
extern bool full;

extern SymEntry *symbol_tbl;
extern int symbol_tbl_size;
extern int call_depth;

extern TailRecNode *tail_rec_head;

// 函数声明
void trace_inst(word_t pc, uint32_t inst);
void display_inst();
void display_pread(paddr_t addr, int len);
void display_pwrite(paddr_t addr, int len, word_t data);
void parse_elf(const char *elf_file);
void trace_func_call(paddr_t pc, paddr_t target, bool is_tail);
void trace_func_ret(paddr_t pc);
void trace_dread(paddr_t addr, int len, IOMap *map);
void trace_dwrite(paddr_t addr, int len, word_t data, IOMap *map);

#endif // ITRACE_H
