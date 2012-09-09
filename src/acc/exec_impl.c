#include "exec.h"
#include "acc/instr.h"
#include "mem.h"

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, acc, arg;

  while(1){
    switch(mem_read8(pc++)){
    case LOAD:
      arg = mem_read32(pc);
      pc += 4;
      acc = mem_read16(data+arg);
      break;

    case STOR:
      arg = mem_read32(pc);
      pc += 4;
      mem_write16(data+arg,acc);
      break;

    case ADD:
      arg = mem_read32(pc);
      pc += 4;
      acc += mem_read16(data+arg);
      break;

    case MUL:
      arg = mem_read32(pc);
      pc += 4;
      acc *= mem_read16(data+arg);
      break;

    case END:
      goto end; //Trololo
    }
  }

 end:
  return 0;
}
