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

  mem_cleanup();
  return 0;
}
