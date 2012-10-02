#include "asm.h"
#include "asm_impl.h"
#include "gpr/instr.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define OPERATOR(x,y) if(strcmp(operator,#x) == 0){ instr->opcode = x; instr->argc = y; }

struct asm_instr* asm_decode_instr(char* operator, int argc, char argv[MAX_ARGC][MAX_TOKEN_LEN]){
  struct asm_instr* instr = (struct asm_instr*)malloc(sizeof(struct asm_instr));

       OPERATOR(END,0)
  else OPERATOR(LA,2)
  else OPERATOR(LB,3)
  else OPERATOR(LI,2)
  else OPERATOR(STO,2)
  else OPERATOR(B,2)
  else OPERATOR(BEQZ,2)
  else OPERATOR(BGE,2)
  else OPERATOR(BNE,2)
  else OPERATOR(ADD,2)
  else OPERATOR(ADDI,2)
  else OPERATOR(SUBI,2)
  else OPERATOR(MUL,2)
  else OPERATOR(SYSCALL,0)

  return instr;
}
