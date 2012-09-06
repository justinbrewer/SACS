#include "asm.h"

int main(int argc, char** argv){
  asm_init();
  struct asm_binary* bin = asm_parse_file(argv[1]);
  asm_free_binary(bin);
  asm_cleanup();
  return 0;
}
