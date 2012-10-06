#include "asm.h"
#include "mem.h"
#include "exec.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, char** argv){
  if(argc != 2){
    printf("Usage: sacs <source-file>\n");
    return 0;
  }

  struct asm_binary* bin = asm_parse_file(argv[1]);

  mem_init();
  mem_dynamic_alloc(0x400000,MEM_FAKE_ALLOC|MEM_USE_LOCKED);
  uint32_t loc = mem_dynamic_alloc(bin->size,MEM_USE_LOCKED);
  assert(loc == 0x400000);
  memcpy(mem_translate_addr(loc),bin->binary,bin->size);

  exec_run(loc,loc,loc+bin->data_segment);

  //Memory Dump
  uint8_t* ptr = (uint8_t*)mem_translate_addr(loc);
  uint32_t i;

  printf("\n=== Memory Dump ===\n.text");
  for(i=0,loc=0x400000; i < bin->data_segment; i++, ptr++){
    if(i%8 == 0){
      printf("\n%#8x: ", loc + i);
    }
    printf("%02x ", *ptr);
  }
  printf("\n");

  printf("\n.data");
  for(loc+=i, i=0; i < bin->size; i++, ptr++){
    if(i%8 == 0){
      printf("\n%#8x: ", loc + i);
    }
    printf("%02x ", *ptr);
  }
  printf("\n");

  mem_cleanup();
  return 0;
}
