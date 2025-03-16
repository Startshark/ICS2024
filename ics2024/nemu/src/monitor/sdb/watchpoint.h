// watchpoint.h
#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include <stdbool.h>
#include <stdint.h>
#include <common.h>
#include "expr.h"

// 定义常量
#define NR_WP 32

// 定义watchpoint结构
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  bool use;
  char expr[1000];
  int old_value;
  int new_value;

} WP;

// 声明全局变量
extern WP wp_pool[NR_WP];
extern WP *head;
extern WP *free_;

// 声明函数
void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);
void display_watchpoint();
void delete_watchpoint(int NO);
void create_watchpoint(char *args, word_t val);
void check_difference();

#endif // WATCHPOINT_H
