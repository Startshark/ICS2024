#include "common.h"
#include <inttypes.h>

void mem_read(uintptr_t block_num, uint8_t *buf);
void mem_write(uintptr_t block_num, const uint8_t *buf);

static uint64_t cycle_cnt = 0;

void cycle_increase(int n) { cycle_cnt += n; }

// TODO: implement the following functions


typedef struct {
  bool valid;
  bool dirty;
  uintptr_t tag;
  uint8_t data[BLOCK_SIZE];
} CacheLine;


static CacheLine* cache;
static uint32_t cache_size_width, cache_associativity_width, cache_group_width;
uintptr_t addr_mask, tag_mask, block_number_mask, group_number_mask;

void init_cache(int total_size_width, int associativity_width) {
  cache_size_width = total_size_width;
  cache_associativity_width = associativity_width;
  cache_group_width = cache_size_width - BLOCK_WIDTH - associativity_width;

  cache = calloc(exp2(cache_size_width) / BLOCK_SIZE, sizeof(CacheLine));
  assert(cache);


  addr_mask = mask_with_len(BLOCK_WIDTH); // addr_mask = 0x3
  group_number_mask = mask_with_len(cache_group_width) << BLOCK_WIDTH; // group_number_mask = 0x3ffc
  tag_mask = ~mask_with_len(BLOCK_WIDTH + cache_group_width) ; // tag_mask = 0xffffc000
  block_number_mask = ~mask_with_len(BLOCK_WIDTH); // block_number_mask = 0xfffffff0

}

static void write(CacheLine* line, uintptr_t addr){ // 写回
  uintptr_t block_num;
  const uint8_t *buf;
  block_num = (line->tag << cache_group_width | ((addr & group_number_mask) >> BLOCK_WIDTH));
  buf = line->data;
  mem_write(block_num, buf);
  line->dirty = false;
}


static void read(uintptr_t addr, uint32_t index){ // 读入数据
  CacheLine* line = &cache[(((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width) + index];
  mem_read(((addr & block_number_mask) >> BLOCK_WIDTH), line->data);
  line->valid = true;
  line->dirty = false;
  line->tag = (addr & tag_mask) >> (BLOCK_WIDTH + cache_group_width);
}



uint32_t cache_read(uintptr_t addr) {
  for(int i = (((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width);
    i < ((((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width) + exp2(cache_associativity_width)); i++) {
    CacheLine* line = &cache[i];
    if (line->valid && line->tag == ((addr & tag_mask) >> (BLOCK_WIDTH + cache_group_width))) {
      cycle_increase(1);
      return *((uint32_t*)&cache[i].data[(addr & addr_mask) & (~0x3)]);
    }
  }

  //cache 缺失
  uint32_t index = rand() % exp2(cache_associativity_width);
  CacheLine* target = &cache[(((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width) + index];
  if(target->dirty) { // 如果该行dirty, 需要写回
    write(target, addr);
  }

  read(addr, index); // 读入数据

  cycle_increase(100); // 100 cycles 只是假定的内存访问时间
  return *(uint32_t*)(&target->data[(addr & addr_mask) & (~0x3)]);

}

void cache_write(uintptr_t addr, uint32_t data, uint32_t wmask) {
  bool cache_hit = false;
  CacheLine* target;

  for(int i = (((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width);
    i < ((((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width) + exp2(cache_associativity_width)); i++) {
      if(cache[i].valid && cache[i].tag == ((addr & tag_mask) >> (BLOCK_WIDTH + cache_group_width))) {
        cache_hit = true;
        target = &cache[i];
        break;
      }
  }
  if(!cache_hit) {
    uint32_t index = rand() % exp2(cache_associativity_width);
    target = &cache[(((addr & group_number_mask) >> BLOCK_WIDTH) << cache_associativity_width) + index];
    if(target->valid){
      if(target->dirty) {
        write(target, addr);
      }
      read(addr, index);
    }
    else{
      read(addr, index);
    }
  }
  uint32_t *p = (uint32_t*)(&target->data[(addr & addr_mask) & (~0x3)]);
  *p = (*p & ~wmask) | (data & wmask);
  target->dirty = true;
  cycle_increase(100); // 100 cycles 只是假定的内存访问时间
}



void display_statistic(void) {
  printf("Total cycles: %" PRIu64 "\n", cycle_cnt);
}
