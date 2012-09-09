#include "asm.h"
#include "asm_impl.h"
#include "stk/instr.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

struct asm_instr* asm_decode_instr(char* operator, int argc, char argv[MAX_ARGC][MAX_TOKEN_LEN]){
  struct asm_instr* instr = (struct asm_instr*)malloc(sizeof(struct asm_instr));
  
  if(strcmp(operator,"end") == 0){
    instr->opcode = END;
    instr->argc = 0;
  }else if(strcmp(operator,"push") == 0){
    instr->opcode = PUSH;
    instr->argc = 1;
  }else if(strcmp(operator,"pop") == 0){
    instr->opcode = POP;
    instr->argc = 1;
  }else if(strcmp(operator,"add") == 0){
    instr->opcode = ADD;
    instr->argc = 0;
  }else if(strcmp(operator,"mul") == 0){
    instr->opcode = MUL;
    instr->argc = 0;
  }else{
    assert(0);
  }

  assert(argc == instr->argc);

  int i;
  for(i=0;i<argc;i++){
    if(isalpha(argv[i][0])){
      instr->argv[i].type = REFERENCE;
      strcpy(instr->argv[i].reference,argv[i]);
    }else{
      //TODO: Handle immediates?
    }
  }

  return instr;
}
