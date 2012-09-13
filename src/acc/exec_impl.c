#include "exec.h"
#include "acc/instr.h"
#include "mem.h"

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, acc, ir;

  while(1){
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

    case END:
      goto end; //Trololo
    }
  }

 end:
  return 0;
}
