#include <common.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <../../../include/device/map.h>

#define MAX_IRINGBUF 16
#define ftrace_write log_write
#define dtrace_write log_write

typedef struct {
  word_t pc;
  uint32_t inst;
} ItraceNode;

static ItraceNode iringbuf[MAX_IRINGBUF];
static int iringbuf_pos = 0;
static bool iringbuf_full = false;

void trace_inst(word_t pc, uint32_t inst) {
  iringbuf[iringbuf_pos].pc = pc;
  iringbuf[iringbuf_pos].inst = inst;
  iringbuf_pos = (iringbuf_pos + 1) % MAX_IRINGBUF;
  iringbuf_full = iringbuf_full || (iringbuf_pos == 0);
}

void display_inst() {
  #ifdef CONFIG_ITRACE
    if (!iringbuf_full && iringbuf_pos == 0) return;

    int end_pos = iringbuf_pos;
    int start_pos = iringbuf_full ? iringbuf_pos : 0;
    
    char buf[128];
    char *p = NULL;
    
    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);

    puts("Most recently executed instructions");
    do {
      p = buf;
      p += snprintf(buf, sizeof(buf), "%s" FMT_WORD ": %08x ", 
                    ((start_pos + 1) % MAX_IRINGBUF == end_pos ? " --> " : "     "),
                    iringbuf[start_pos].pc, iringbuf[start_pos].inst);
      
      disassemble(p, buf + sizeof(buf) - p, iringbuf[start_pos].pc, 
                  (uint8_t *)&iringbuf[start_pos].inst, 4);

      if ((start_pos + 1) % MAX_IRINGBUF == end_pos) printf(ANSI_FG_RED);
      puts(buf);
    } while ((start_pos = (start_pos + 1) % MAX_IRINGBUF) != end_pos);
    puts(ANSI_NONE);
  #endif
}

void display_pread(paddr_t addr, int len) {
  printf("pread at " FMT_PADDR " len=%d\n", addr, len);
}

void display_pwrite(paddr_t addr, int len, word_t data) {
  printf("pwrite at " FMT_PADDR " len=%d, data=" FMT_WORD "\n", addr, len, data);
}

typedef struct {
  char name[32];
  paddr_t addr;
  unsigned char info;
  Elf64_Xword size;
} SymEntry;

static SymEntry *symbol_table = NULL;
static int symbol_table_size = 0;
static int call_stack_depth = 0;

typedef struct TailRecNode {
  paddr_t pc;
  paddr_t dependency;
  struct TailRecNode *next;
} TailRecNode;

static TailRecNode *tail_recursion_head = NULL;

// static void init_tail_recursion_list() {
//   tail_recursion_head = malloc(sizeof(TailRecNode));
//   assert(tail_recursion_head != NULL);
//   tail_recursion_head->pc = 0;
//   tail_recursion_head->next = NULL;
// }

static int locate_function_symbol(paddr_t addr, bool is_call) {
  for (int i = 0; i < symbol_table_size; ++i) {
    if (ELF64_ST_TYPE(symbol_table[i].info) == STT_FUNC) {
      if ((is_call && symbol_table[i].addr == addr) || 
          (!is_call && symbol_table[i].addr <= addr && addr < symbol_table[i].addr + symbol_table[i].size)) {
        return i;
      }
    }
  }
  return -1;
}

static void handle_tail_recursion_insert(paddr_t pc, paddr_t dependency) {
  TailRecNode *node = malloc(sizeof(TailRecNode));
  assert(node != NULL);
  node->pc = pc;
  node->dependency = dependency;
  node->next = tail_recursion_head->next;
  tail_recursion_head->next = node;
}

static void handle_tail_recursion_remove() {
  TailRecNode *node = tail_recursion_head->next;
  if (node != NULL) {
    tail_recursion_head->next = node->next;
    free(node);
  }
}

void trace_func_call(paddr_t pc, paddr_t target, bool is_tail) {
  if (symbol_table == NULL) return;

  ++call_stack_depth;

  if (call_stack_depth <= 2) return;

  int symbol_idx = locate_function_symbol(target, true);
  ftrace_write(FMT_PADDR ": %*scall [%s@" FMT_PADDR "]\n",
               pc,
               (call_stack_depth - 3) * 2, "",
               (symbol_idx >= 0) ? symbol_table[symbol_idx].name : "???",
               target);

  if (is_tail) {
    handle_tail_recursion_insert(pc, target);
  }
}

void trace_func_ret(paddr_t pc) {
  if (symbol_table == NULL) return;

  if (call_stack_depth <= 2) return;

  int symbol_idx = locate_function_symbol(pc, false);
  ftrace_write(FMT_PADDR ": %*sret [%s]\n",
               pc,
               (call_stack_depth - 3) * 2, "",
               (symbol_idx >= 0) ? symbol_table[symbol_idx].name : "???");

  --call_stack_depth;

  TailRecNode *node = tail_recursion_head->next;
  if (node != NULL) {
    int dependency_idx = locate_function_symbol(node->dependency, true);
    if (dependency_idx == symbol_idx) {
      paddr_t ret_pc = node->pc;
      handle_tail_recursion_remove();
      trace_func_ret(ret_pc);
    }
  }
}

void trace_dread(paddr_t addr, int len, IOMap *map) {
  dtrace_write("dtrace: read %10s at " FMT_PADDR ",%d\n", map->name, addr, len);
}

void trace_dwrite(paddr_t addr, int len, word_t data, IOMap *map) {
  dtrace_write("dtrace: write %10s at " FMT_PADDR ",%d with " FMT_WORD "\n", 
               map->name, addr, len, data);
}
