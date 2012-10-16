#include "asm.h"
#include "asm_impl.h"
#include "instr.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define CHECK_ARGC							\
  if(instr->argc != argc){						\
    printf("Error: %s takes %d arguments, got %d\n",			\
	   operator,instr->argc,argc);					\
    exit(EXIT_FAILURE);							\
  }

#define REGISTER_ARG(i)						\
  instr->argv[i].type = VALUE;					\
  instr->argv[i].value = _translate_reg_name(argv[i]);

#define LABEL_ARG(i)				\
  instr->argv[i].type = REFERENCE;		\
  strcpy(instr->argv[i].reference,argv[i]);

#define IMM_ARG(i)				\
  instr->argv[i].type = VALUE;			\
  instr->argv[i].value = atoi(argv[i]);

uint32_t _translate_reg_name(char name[MAX_TOKEN_LEN]);

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

    REGISTER_ARG(0);
    LABEL_ARG(1);
  }

  //LB requires special processing, since one arg is passed as imm(reg)
  else if(strcmp(operator,"lb") == 0){
    instr->opcode = LB;
    instr->argc = 3;

    if(argc != 2){
      printf("Error: %s takes %d arguments, got %d\n",
	     operator,2,argc);
      exit(EXIT_FAILURE);
    }

    //Split "imm(reg)" into "imm" "reg"
    int i, len = strlen(argv[1]);
    char* reg;
    for(i=0;i<len;i++){
      if(argv[1][i] == '('){
	argv[1][i] = 0;
	reg = (&argv[1][i])+1;
	continue;
      }
      if(argv[1][i] == ')'){
	argv[1][i] = 0;
	break;
      }
    }
    strcpy(argv[2],reg);

    REGISTER_ARG(0);
    IMM_ARG(1);
    REGISTER_ARG(2);
  }

  else if(strcmp(operator,"li") == 0){
    instr->opcode = LI;
    instr->argc = 2;
    CHECK_ARGC;

    REGISTER_ARG(0);
    IMM_ARG(1);
  }

  else if(strcmp(operator,"b") == 0){
    instr->opcode = B;
    instr->argc = 1;
    CHECK_ARGC;

    LABEL_ARG(0);
  }

  else if(strcmp(operator,"beqz") == 0){
    instr->opcode = BEQZ;
    instr->argc = 2;
    CHECK_ARGC;

    REGISTER_ARG(0);
    LABEL_ARG(1);
  }

  else if(strcmp(operator,"bge") == 0){
    instr->opcode = BGE;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    LABEL_ARG(2);
  }

  else if(strcmp(operator,"bne") == 0){
    instr->opcode = BNE;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    LABEL_ARG(2);
  }

  else if(strcmp(operator,"addi") == 0){
    instr->opcode = ADDI;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    IMM_ARG(2);
  }

  else if(strcmp(operator,"subi") == 0){
    instr->opcode = SUBI;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    IMM_ARG(2);
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

uint32_t _translate_reg_name(char name[MAX_TOKEN_LEN]){
  int i;
  static char register_names[32][5] = {"zero","at","v0","v1","a0","a1","a2","a3",
                                       "t0","t1","t2","t3","t4","t5","t6","t7",
                                       "s0","s1","s2","s3","s4","s5","s6","s7",
                                       "t8","t9","k0","k1","gp","sp","fp","ra"};

  if(name[0] != '$'){
    printf("Error: Expected register name, got \"%s\"\n",name);
    exit(EXIT_FAILURE);
  }

  if(isdigit(name[1])){
    return atoi(name+1);
  }

  for(i=0;i<32;i++){
    if(strcmp(name+1,register_names[i]) == 0){
      break;
    }
  }
  if(i < 32){
    return i;
  }

  printf("Error: Unknown register \"%s\"\n",name);
  exit(EXIT_FAILURE);
}
