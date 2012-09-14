#include "exec.h"
#include "acc/instr.h"
#include "mem.h"

#include <stdio.h>

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, acc, ir, running = 1;

  while(running){
    ir = mem_read32(pc);
    pc += 4;

    switch(ir & 0xFF){
    case LOAD:
      acc = mem_read32(data + (ir >> 8));
      break;

    case STOR:
      mem_write32(data + (ir >> 8),acc);
      break;

    case ADD:
      acc += mem_read32(data + (ir >> 8));
      break;

    case MUL:
      acc *= mem_read32(data + (ir >> 8));
      break;

    case PRNT:
      printf("0x%x: 0x%x\n",(data + (ir >> 8)),mem_read32(data + (ir >> 8)));
      break;

    case END:
      running = 0;
      break;
    }
  }

  return 0;
}
