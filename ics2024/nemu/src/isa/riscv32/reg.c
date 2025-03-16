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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  int reg_num = ARRLEN(regs);
  int i;

  for (i = 0; i < reg_num; i++) {
    printf("%-8s%-#20x%-20u\n", regs[i], cpu.gpr[i], cpu.gpr[i]);
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if(strcmp(s, regs[0]) == 0){
      *success = true;
      // printf("Reg Str transformation success.\n");
      return (word_t)cpu.gpr[0];
    }
  for(int i = 0; i < 32; i++){
    char prefix[10];
    sprintf(prefix, "$%s", regs[i]);  // 添加“$”前缀以供识别。
    if(strcmp(s, prefix) == 0){
      *success = true;
      // printf("Reg Str transformation success.\n");
      return (word_t)cpu.gpr[i];
    }
  }
  *success = false;
  // printf("You shouldn't go this way!\n");
  return 0;
}

/*
0x80000000: 00000297
0x80000004: 00028823
0x80000008: 0102c503
0x8000000c: 00100073
0x80000010: deadbeef
0x80000014: 12121212
0x80000018: 12121212
0x8000001c: 12121212
0x80000020: 12121212
0x80000024: 12121212
*/
