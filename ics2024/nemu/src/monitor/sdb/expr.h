#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdbool.h>
#include <stdint.h>
#include <isa.h>
#include <regex.h>

void init_regex(void);

word_t expr(char *e, bool *success);

#endif // __EXPR_H__
