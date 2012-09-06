#include "asm.h"
#include "asm_impl.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

struct asm_instr* asm_decode_instr(char* operator, int argc, char** argv){
  struct asm_intsr* instr = (struct asm_instr*)malloc(sizeof(struct asm_instr));
  
  if(strcmp(operator,"end")){
    instr->opcode = 0x00;
    instr->argc = 0;
  }else if(strcmp(operator,"load")){
    instr->opcode = 0x01;
    instr->argc = 1;
  }else if(strcmp(operator,"stor")){
    instr->opcode = 0x02;
    instr->argc = 1;
  }else if(strcmp(operator,"add")){
    instr->opcode = 0x03;
    instr->argc = 1;
  }else if(strcmp(operator,"mul")){
    instr->opcode = 0x04;
    instr->argc = 1;
  }else{
    assert(0);
  }

  assert(argc == instr->argc);

  for(int i=0;i<argc;i++){
    if(isalpha(argv[i][0])){
      instr->argv[i].type = REFERENCE;
      strcpy(instr->argv[i].data.reference,argv[i]);
    }else{
      //TODO: Handle immediates?
    }
  }

  return instr;
}
