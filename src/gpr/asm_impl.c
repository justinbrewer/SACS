#include "asm.h"
#include "asm_impl.h"
#include "gpr/instr.h"
#include "check.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CHECK_ARGC if(instr->argc != argc){printf("Error: %s takes %d arguments\n",operator,instr->argc);exit(EXIT_FAILURE);}

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
