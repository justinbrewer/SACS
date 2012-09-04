#include "asm.h"
#include "asm_impl.h"

#include <stdlib.h>
#include <string.h>

int asm_init(){return 0;}
int asm_cleanup(){return 0;}

struct asm_binary* asm_parse_file(char* file){}

struct asm_binary* asm_parse(int lines, char** text){}

int asm_free_binary(struct asm_binary* bin){
  return _delete_binary(bin);
}

struct asm_binary* _create_binary(){
  struct asm_binary* bin = (struct asm_binary*)malloc(sizeof(struct asm_binary));
  memset(bin,0,sizeof(struct asm_binary));
  return bin;
}

int _delete_binary(struct asm_binary* bin){
  if(bin->binary != 0){
    free(bin->binary);
  }
  free(bin);
}

struct asm_instr* _create_instr(){}
int _delete_instr(struct asm_instr* instr){}

struct asm_arg* _create_arg(uint8_t max_argc0){}
int _delete_arg(struct asm_arg* arg){}
