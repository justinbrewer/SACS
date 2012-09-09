#include "asm.h"
#include "mem.h"

#include <stdio.h>

int main(int argc, char** argv){
  if(argc != 1){
    printf("Usage: sacs <source-file>\n");
    return 0;
  }

  asm_init();
  struct asm_binary* bin = asm_parse_file(argv[1]);

  mem_init();



  mem_cleanup();
  asm_cleanup();
  return 0;
}
