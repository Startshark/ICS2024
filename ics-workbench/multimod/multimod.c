#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#define MAX ((uint64_t)(-1))

// Version 3.0
static uint64_t bit_of(uint64_t x, int i) {
    return (x >> i) & 1;
}

static uint64_t mod(uint64_t a, uint64_t b){
    // 实现 a % b
    if(b == 0){
        return 0;
    }
    uint64_t multiple = b;
    while(a >= multiple && (multiple << 1) > multiple)  multiple <<= 1;
    while(a >= b){
        while(multiple > a) multiple >>= 1;
        a -= multiple;
    }
    return a;
}

static uint64_t modadd(uint64_t x, uint64_t y, uint64_t m){
    // 实现 (x + y) % m
    uint64_t sum = x + y;
    if (sum < x) { // 检测溢出
        sum -= m;  // 模掉 m
    }
    return mod(sum,m);
}

static uint64_t modmul(uint64_t x, uint64_t y, uint64_t m) {
    // 实现 (x * y) % m，使用迭代避免递归, 这里将y做二进制展开(方便后面 1ULL<<i 处理)
    uint64_t result = 0;
    uint64_t addend = mod(x, m);

    while (y > 0) {
        if (y & 1) {  // 如果 y 的最低位为 1
            result = modadd(result, addend, m);
        }
        addend = modadd(addend, addend, m);  // 左移addend
        y >>= 1;  // 右移 y
    }

    return result;
}

uint64_t multimod(uint64_t a, uint64_t b, uint64_t m) {
    uint64_t ans = 0;
    for (int i = 0; i < 64; i++) {
        if (bit_of(a, i)) {
            ans = modadd(ans, modmul(b, (1ULL << i), m), m);
        }
    }
    return ans;
}

// Test
// int main() {
//     uint64_t test = multimod(123456789ULL, 987654321ULL, 1000000007ULL);
//     printf("%lu\n", test);
//     return 0;
// }
