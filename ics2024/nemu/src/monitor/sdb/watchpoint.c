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

#include "expr.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  // bool use;
  char expr[1000];
  int old_val;
} WP;

WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static WP* new_wp(){
  assert(free_);
  WP* wp = free_;
  free_ = free_ -> next;
  // wp->use = true;
  wp -> next = head;
  head = wp;
  return wp;
}

static void free_wp(WP *wp, bool *success){
  WP* p = head;
  if(p == wp){ 
    head = head->next;
    p->next = free_;
    free_ = p;
  }
  else{
    while(p && p->next != wp) p = p->next;
    if(!p){
      printf("Trying to Free Invalid Watchpoint!\n");
      *success = false;
      return;
    }
    p->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
}

void create_watchpoint(char *expr, word_t val){
  WP* p = new_wp();
  strcpy(p->expr, expr);
  p->old_val = val;
  printf("Watchpoint %d: '%s' created successfully.\n", p->NO, expr);
}

void delete_watchpoint(int NO){
  assert(NO < NR_WP);
  WP* p = &wp_pool[NO];
  bool success = true;
  free_wp(p, &success);
  if(success){
    printf("Delete watchpoint %d: '%s' successfully.\n", p->NO, p->expr);
  }
}

void display_watchpoint(){
  WP *p = head;
  if(!p){
    printf("No watchpoints now.\n");
    return ;
  }
  while(p){
    printf("Watchpoint %d:  '%s' and the value is %u\n", p->NO, p->expr, p->old_val);
    p = p->next;
  }
}

void check_difference(){
  WP* p = head;
  while(p){
    bool ok;
    word_t new_val = expr(p->expr, &ok);
    if(p->old_val != new_val){
      printf("Watchpoint %d:  %s:\n"
      "Old value is %u\n"
      "New value is %u\n", p->NO, p->expr, p->old_val, new_val);
      p->old_val = new_val;
    }
    p = p->next;
  }
}