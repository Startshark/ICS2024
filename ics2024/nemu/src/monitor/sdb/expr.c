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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include "../../memory/vaddr.h"

enum {
  TK_NOTYPE = 256,
  TK_POS, TK_NEG, TK_DEREF,
  /* 比较 */
  TK_EQ, TK_NEQ, TK_LT, TK_GT, TK_LEQ, TK_GEQ,
  TK_AND, TK_OR,
  /* 运算数以及寄存器 */
  TK_NUM, TK_REG, TK_PC
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE}, 
  /* 按照优先级排序，方便后面find_major函数的处理 */
  {"\\(", '('}, {"\\)", ')'},
  {"\\*", '*'}, {"/", '/'},
  {"\\+", '+'}, {"-", '-'},
  {"<", TK_LT}, {">", TK_GT}, {"<=", TK_LEQ}, {">=", TK_GEQ},
  {"==", TK_EQ}, {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"([0-9]+|0x[0-9a-fA-F]+)", TK_NUM},  // 包括16进制数，发现其实可以一起处理, 但是别忘了16进制数有a-f！(最后一错！)
  {"\\$(0|\\$0|ra|sp|gp|tp|t[0-6]|s(10|11|[0-9])|a[0-7])", TK_REG},
  {"\\$pc", TK_PC}
};


#define NR_REGEX ARRLEN(rules)
#define INTYPES(type, types) intypes(type, types, ARRLEN(types))

static int btypes[] = {')',TK_NUM,TK_REG}; 
static int ntypes[] = {'(',')',TK_NUM,TK_REG}; 
static int untypes[] = {TK_NEG, TK_POS, TK_DEREF};

static bool intypes(int t, int types[], int s) {
  for (int i = 0; i < s; i++) {
    if (t == types[i]) return true;
  }
  return false;
}

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

// int charint(char s[]){
//     int size = strlen(s);
//     int res = 0;
//     for(int i = 0 ; i < size ; i ++)
//     {
//       res += s[i] - '0';
//       res *= 10;
//     }
//     res /= 10;
//     return res;
// }

// void intchar(int x, char str[]){
//     int len = strlen(str);
//     memset(str, 0, len);
//     int tmp_index = 0;
//     int tmp = x;
//     int flag = 1;
//     while(tmp){
//       tmp /= 10;
//       flag *= 10;
//     }
//     flag /= 10;
//     while(x)
//     {
//       int a = x / flag; 
//       x %= flag;
//       flag /= 10;
//       str[tmp_index ++] = a + '0';
//     }
// }

static Token tokens[1000] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        /*
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        */

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(rules[i].token_type == TK_NOTYPE) break;
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case TK_NUM:
            // printf("Num Recognized!\n"); 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          case TK_REG:
            // printf("Register recognized!\n"); // DeBug!
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          case '*': case '-': case '+':
            if(nr_token == 0 || !INTYPES(tokens[nr_token-1].type, btypes)){
              switch(rules[i].token_type){
                case '-': tokens[nr_token].type = TK_NEG; break;
                case '+': tokens[nr_token].type = TK_POS; break;
                case '*': tokens[nr_token].type = TK_DEREF; break;
              }
            }
          /*
            default:
            printf("Error, i = %d and No rules fit.", i);
            break;
          */
        }
        nr_token++;
        break;
      }
    }

    /* TODO: Handle the negative number and HEX. */
    

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

// 检查括号匹配
bool check_parentheses(int p, int q){
  if(tokens[p].type == '(' && tokens[q].type == ')'){
    int left = 0;
    for(int i = p; i <= q; i++){
      if(tokens[i].type == '(') left++;
      else if(tokens[i].type == ')') left--;

      if(left == 0) return i==q; 
    }
  }
  return false;
}

static uint32_t operand(int i, bool *ok){
  *ok = true;
  switch(tokens[i].type){
    case TK_NUM:{
      if (strncmp("0x", tokens[i].str, 2) == 0) return strtol(tokens[i].str, NULL, 16);  // HEX
      else return strtol(tokens[i].str, NULL, 10);
      break; 
    }
    case TK_REG:  return isa_reg_str2val(tokens[i].str, ok);  break;
    case TK_PC:  return cpu.pc; break;
    default:{
      *ok = false;
      return 0;
      break;
    }
  }
}

/* 单目操作符 */
static uint32_t calc1(int op, uint32_t val, bool *ok){
  switch(op){
    case TK_NEG: return -val;
    case TK_POS: return val;
    case TK_DEREF: return vaddr_read(val, 4);
    default: *ok = false;
    }
  return 0;
}

/* 多目操作符 */
static uint32_t calc2(uint32_t v1, int op, uint32_t v2, bool *ok){
  switch(op){
    case '+': return v1 + v2;
    case '-': return v1 - v2;
    case '*': return v1 * v2;
    case '/': {
      if(v2 == 0){
        printf("Divisor is zero!\n");
        *ok = false;
        return 0;
      }
      return (sword_t)v1 / (sword_t)v2;
    }
  case TK_AND: return v1 && v2;
  case TK_OR: return v1 || v2;
  case TK_EQ: return v1 == v2;
  case TK_NEQ: return v1 != v2;
  case TK_GT: return v1 > v2;
  case TK_LT: return v1 < v2;
  case TK_GEQ: return v1 >= v2;
  case TK_LEQ: return v1 <= v2;
  default:{
    *ok = false; 
    printf("Operand not found.");
    return 0;
  }
    return 0;
  }
}

int find_major_op(int p, int q) {
  /* left 变量记录当前位置之前有多少个未匹配的左括号 */
  int op_pos = -1;
  int left = 0, op_priority = 0;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == '(')  left ++;
    else if(tokens[i].type == ')'){
      if(left == 0)  return -1;
      left--;
    }
    else if(INTYPES(tokens[i].type, ntypes) || left > 0)  continue;
    else{
      int tmp_pre = 0;
      switch(tokens[i].type){
        /* 无需break， 因为加起来的就是优先级 */
        case TK_OR: tmp_pre++;
        case TK_AND: tmp_pre++;
        case TK_EQ: case TK_NEQ: tmp_pre++;
        case TK_LT: case TK_GT: case TK_GEQ: case TK_LEQ: tmp_pre++;
        case '+': case '-': tmp_pre++;
        case '*': case '/': tmp_pre++;
        case TK_NEG: case TK_DEREF: case TK_POS: tmp_pre++;  break;
        default:{
            printf("You shouldn't be here.");
            return -1;
          }
        }
        if(tmp_pre > op_priority ||  (tmp_pre == op_priority && \
        !INTYPES(tokens[i].type, untypes))){
          op_priority = tmp_pre;
          op_pos = i;
        }
      }
    }
  if (left != 0) return -1;
  if(op_pos < 0) printf("Error!\n");
  // printf("Op position is %d\n", op_pos);  // Debug!
  return op_pos;
}


uint32_t eval(int p, int q, bool *ok) {
  *ok = true;  // We take the input valid initally.
  if(p > q){
    *ok = false;
    return 0;
  } 
  else if(p == q){
    /* If not a number this case, then the expr is invalid. */
    return operand(p, ok);
    }
  else if(check_parentheses(p, q)){
    return eval(p + 1, q - 1, ok);
  } 
  else{
    int major = find_major_op(p, q);  // The major op in the expr
    //if(major == 0 && tokens[major].type == TK_NEG){}
    if(major < 0){
      *ok = false;
      return 0;
    }
    bool ok1 = true, ok2 = true;
    uint32_t v1 = eval(p, major-1, &ok1);
    uint32_t v2 = eval(major+1, q, &ok2);
    if(!ok2){
      *ok = false;
      return 0;
    }
    if(ok1){
      uint32_t res = calc2(v1, tokens[major].type, v2, ok);
      return res;
    }
    else{
      uint32_t res = calc1(tokens[major].type, v2, ok);
      return res;
    }

    switch(tokens[major].type){
      case '+': return v1 + v2;
      case '-': return v1 - v2;
      case '*': return v1 * v2;
      case '/': {
        if(v2 == 0){
          *ok = false;
          return 0;
        } 
        return (sword_t)v1 / (sword_t)v2;
      }
      default: assert(0);
    }
  }
}


word_t expr(char *e, bool *success){
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  word_t ans = eval(0, nr_token - 1, success);
  return ans;
}
