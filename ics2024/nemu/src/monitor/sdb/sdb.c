/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/vaddr.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "expr.h"
#include "watchpoint.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_d(char *args);
static int cmd_w(char *args);



static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
	{"si", "Single Step Execution", cmd_si},
  {"info", "Register or Watchpoint detailed infomation", cmd_info},
  {"x", "Scan the memory", cmd_x},
  {"p", "Calculate the value of expr", cmd_p},
  {"w", "Set watchpoint for EXPR", cmd_w},
  {"d", "Delete watchpoint", cmd_d}
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	int steps = 0;
	if(arg == NULL){
		steps = 1;
	}
	else{
    steps = strtol(arg, NULL, 10);
  }
	cpu_exec(steps);
	return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " ");
  /* 看是否是查询寄存器操作 */ 
  if(strcmp(arg, "r") == 0)
  {
    isa_reg_display();
  }
  /* 看是否是查询监视点操作 */ 
  else if(strcmp(arg, "w") == 0)
  {
    display_watchpoint();
  }
  return 0;
}

static int cmd_d(char *args){
  char *arg = strtok(NULL, "");
  if(!arg){
    printf("Usage: d N\n");
    return 0;
  }
  int no = strtol(arg, NULL, 10);
  delete_watchpoint(no);
  return 0;
}

static int cmd_w(char *args){
  if(!args){
    printf("Usage: w EXPR\n");
    return 0;
  }
  bool ok;
  word_t res = expr(args, &ok);
  if(!ok){
    printf("Invalid!");
  }
  else{
    create_watchpoint(args, res);
  }
  return 0;
}

static int cmd_x(char *args){
  char * N = strtok(NULL, " ");
  char * EXPR = strtok(NULL, " ");
  int len;
  vaddr_t address;
  /* 上述得到的N和EXPR均是字符串， 用sscanf转成十进制和十六进制 */
  sscanf(N, "%d", &len);
  sscanf(EXPR, "%x", &address);
  for(int i = 0; i < len; i++){
    printf("0x%x: %08x\n", address, vaddr_read(address, 4));
    address += 4;
  }
  return 0;
}

static int cmd_p(char *args){
  if(args == NULL){
    printf("No args.\n");
    return 0;
  }
  bool success = true;
  uint32_t res = expr(args, &success);
  if(!success) puts("Invalid!\n");
  else printf("%u\n", res);
  return 0;
}

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void test_expr() {
  FILE *fp = fopen("/home/startshark/Desktop/repo/ics2024/nemu/tools/gen-expr/input", "r");
  if (fp == NULL) perror("test_expr error");

  char *e = NULL;
  uint32_t correct_res;
  size_t len = 0;
  size_t read;
  bool success = false;

  while (true) {
    if(fscanf(fp, "%u ", &correct_res) == -1) break;
    read = getline(&e, &len, fp);
    e[read-1] = '\0';
    
    uint32_t res = expr(e, &success);
    
    assert(success);
    if (res != correct_res) {
      puts(e);
      printf("expected: %u, got: %u\n", correct_res, res);
      assert(0);
    }
  }

  fclose(fp);
  if (e) free(e);

  Log("expr test pass");
}


void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();
  
  /* Test math expression */
  // test_expr();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
