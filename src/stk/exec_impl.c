#include "exec.h"
#include "stk/instr.h"
#include "mem.h"

#define STACK_SIZE 32

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, arg, sp = 0, stack[STACK_SIZE]={0};

  while(1){
    switch(mem_read8(pc++)){
    case PUSH:
      arg = mem_read32(pc);
      pc += 4;
      stack[sp++] = mem_read16(data+arg);
      break;

    case POP:
      arg = mem_read32(pc);
      pc += 4;
      mem_write16(data+arg,stack[--sp]);
      break;

    case ADD:
      --sp;
      stack[sp-1] = stack[sp] + stack[sp-1];
      break;

    case MUL:
      --sp;
      stack[sp-1] = stack[sp] * stack[sp-1];
      break;

    case END:
      goto end; //Trololo
    }
  }

 end:
  return 0;
}
