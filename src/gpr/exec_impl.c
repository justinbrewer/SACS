#include "exec.h"
#include "gpr/instr.h"
#include "mem.h"

#include <stdio.h>
#include <string.h>

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  int i;
  char *str;
  uint32_t pc = start, running = 1, reg[32] = {0};
  union gpr_instr_t ir;

  while(running){
    ir.u = mem_read32(pc);
    pc += 4;

    switch(ir.r.op){

    case SYSCALL:
      switch(reg[2]){
      case 4: //Print string
	printf("%s",(char*)mem_translate_addr(reg[4]));
	break;

      case 8: //Read string
	str = (char*)mem_translate_addr(reg[4]);
	scanf("%s",str);

	//scanf does not incude the newline character, but we need it
	i = strlen(str);
	str[i] = '\n';
	str[i+1] = 0;
	break;

      case 10: //Exit
	running = 0;
	break;
      }

      break;

    case LA: //Load address
      reg[ir.i.rd] = data + ir.i.offset;
      break;

    case LB: //Load byte
      reg[ir.i.rd] = mem_read8(reg[ir.i.rs] + ir.i.offset);
      break;

    case LI: //Load immediate
      reg[ir.i.rd] = ir.i.offset;
      break;

    case B: //Branch
      pc = text + ir.j.addr;
      break;

    case BEQZ: //Branch if rs = 0
      if(reg[ir.i.rs] == 0){
	pc = text + ir.i.offset;
      }
      break;

    case BGE: //Branch if rd >= rs
      if(reg[ir.i.rd] >= reg[ir.i.rs]){
	pc = text + ir.i.offset;
      }
      break;

    case BNE: //Branch if rd != rs
      if(reg[ir.i.rd] != reg[ir.i.rs]){
	pc = text + ir.i.offset;
      }
      break;

    case ADDI: //rd = rs + imm
      reg[ir.i.rd] = reg[ir.i.rs] + ir.i.offset;
      break;

    case SUBI: //rd = rs - imm
      reg[ir.i.rd] = reg[ir.i.rs] - ir.i.offset;
      break;
    }

    reg[0] = 0; //This sucks
  }
}
