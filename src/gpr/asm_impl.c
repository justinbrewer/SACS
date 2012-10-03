#include "asm.h"
#include "asm_impl.h"
#include "gpr/instr.h"
#include "check.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CHECK_ARGC							\
  if(instr->argc != argc){						\
    printf("Error: %s takes %d arguments\n",operator,instr->argc);	\
    exit(EXIT_FAILURE);							\
  }

uint32_t _register_arg(char name[MAX_TOKEN_LEN]);

struct asm_instr* asm_decode_instr(char* operator, int argc, char argv[MAX_ARGC][MAX_TOKEN_LEN]){
  struct asm_instr* instr = (struct asm_instr*)malloc(sizeof(struct asm_instr));

  if(strcmp(operator,"syscall") == 0){
    instr->opcode = SYSCALL;
    instr->argc = 0;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"la") == 0){
    instr->opcode = LA;
    instr->argc = 2;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"lb") == 0){
    instr->opcode = LB;
    instr->argc = 2;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"li") == 0){
    instr->opcode = LI;
    instr->argc = 2;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"b") == 0){
    instr->opcode = B;
    instr->argc = 1;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"beqz") == 0){
    instr->opcode = BEQZ;
    instr->argc = 2;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"bge") == 0){
    instr->opcode = BGE;
    instr->argc = 3;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"bne") == 0){
    instr->opcode = BNE;
    instr->argc = 3;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"addi") == 0){
    instr->opcode = ADDI;
    instr->argc = 3;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"subi") == 0){
    instr->opcode = SUBI;
    instr->argc = 3;
    CHECK_ARGC;
  }

  else {
    printf("Error: Unknown argument \"%s\"\n",operator);
    exit(EXIT_FAILURE);
  }

  return instr;
}

uint32_t asm_collapse_instr(struct asm_instr* instr){
  union gpr_instr_t res;
  res.u = 0;

  switch(instr->opcode){
  case SYSCALL:
    res.j.op = instr->opcode;
    break;

  case ADDI:
  case BGE:
  case BNE:
  case SUBI:
    res.i.op = instr->opcode;
    res.i.rs = instr->argv[1].value;
    res.i.rd = instr->argv[0].value;
    res.i.offset = instr->argv[2].value;
    break;

  case B:
    res.j.op = instr->opcode;
    res.j.offset = instr->argv[0].value;
    break;

  case BEQZ:
    res.i.op = instr->opcode;
    res.i.rs = instr->argv[0].value;
    res.i.offset = instr->argv[1].value;
    break;

  case LA:
  case LI:
    res.i.op = instr->opcode;
    res.i.rd = instr->argv[0].value;
    res.i.offset = instr->argv[1].value;
    break;

  case LB:
    res.i.op = instr->opcode;
    res.i.rs = instr->argv[2].value;
    res.i.rd = instr->argv[0].value;
    res.i.offset = instr->argv[1].value;
    break;
  }

  return res.u;
}

uint32_t _register_arg(char name[MAX_TOKEN_LEN]){
  int offset;

  if(name[0] != '$'){
    printf("Error: Expected register, got \"%s\"\n",name);
    exit(EXIT_FAILURE);
  }

  switch(name[1]){
  case 't':
    offset = name[2] - 0x30;
    if(offset > 7){
      return offset + 16;
    }
    return offset + 8;

  case 's':
    offset = name[2] - 0x30;
    return offset + 16;
  }
}
