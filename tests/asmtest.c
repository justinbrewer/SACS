#include "asm.h"

int main(int argc, char** argv){
  struct asm_binary* bin = asm_parse_file(argv[1]);
  asm_free_binary(bin);
  return 0;
}
