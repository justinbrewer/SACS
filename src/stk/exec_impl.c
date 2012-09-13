#include "exec.h"
#include "stk/instr.h"
#include "mem.h"

#define STACK_SIZE 32

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, ir, sp = 0, stack[STACK_SIZE]={0};

  while(1){
    ir = mem_read32(pc);
    pc += 4;

    switch(ir & 0xFF){
    case PUSH:
      stack[sp++] = mem_read32(data + (ir >> 8));
      break;

    case POP:
      mem_write32(data + (ir >> 8),stack[--sp]);
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
