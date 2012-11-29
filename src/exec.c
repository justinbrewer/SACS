#include "exec.h"
#include "instr.h"
#include "mem.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_UNITS 5

//Raw conversion to float
#define FLOAT(x) (*(float*)(&x))

typedef enum { FALSE=0, TRUE=1 } bool;
typedef enum { U_NONE=-1, U_INT=0, U_FPADD=1, U_FPMUL=2, U_FPDIV=3, U_MEM=4 } exec_funit_t;

struct exec_funit_state_t {
  uint32_t cycles;

  uint32_t time;
  gpr_op_t op;
  uint32_t rd;
  uint32_t rs;
  uint32_t rt;
  bool rs_r;
  bool rt_r;
};

struct exec_state_t {
  uint32_t pc;
  uint32_t text;
  uint32_t data;

  int running;

  uint32_t reg[64];

  struct exec_funit_state_t funit_state[NUM_UNITS];
  exec_funit_t reg_status[64];

  struct exec_stats_t stats;
};

void exec_issue(struct exec_state_t* current, struct exec_state_t* next);
void exec_read(struct exec_state_t* current, struct exec_state_t* next);
void exec_units(struct exec_state_t* current, struct exec_state_t* next);
void exec_write(struct exec_state_t* current, struct exec_state_t* next);

struct exec_stats_t* exec_run(uint32_t start, uint32_t text, uint32_t data){
  int i;
  struct exec_state_t current = {0}, next;

  current.running = 1;
  current.pc = start;
  current.text = text;
  current.data = data;

  current.funit_state[U_INT].cycles = 1;
  current.funit_state[U_FPADD].cycles = 10;
  current.funit_state[U_FPMUL].cycles = 20;
  current.funit_state[U_FPDIV].cycles = 40;
  current.funit_state[U_MEM].cycles = 1;

  for(i=0;i<64;i++) current.reg_status[i] = U_NONE;

  memcpy(&next,&current,sizeof(struct exec_state_t));

  while(current.running){
    exec_issue(&current,&next);
    exec_read(&current,&next);
    exec_units(&current,&next);
    exec_write(&current,&next);

    memcpy(&current,&next,sizeof(struct exec_state_t));
  }

  struct exec_stats_t* stats = (struct exec_stats_t*)malloc(sizeof(struct exec_stats_t));
  memcpy(stats,&current.stats,sizeof(struct exec_stats_t));
  return stats;
}

void exec_issue(struct exec_state_t* current, struct exec_state_t* next){
  int rd;
  exec_funit_t funit;
  union gpr_instr_t instr;

  instr.u = mem_read32(current->pc);
  next->pc = current->pc + 4;

  switch(instr.j.op){
  case NOP:
    return;

  case SYSCALL:
  case B:
  case BEQZ:
  case BGE:
  case BNE:
    funit = U_INT;
    rd = 0;
    break;

  case ADD:
  case SUB:
    funit = U_INT;
    rd = instr.r.rd;
    break;

  case ADDI:
  case SUBI:
  case LA:
  case LI:
    funit = U_INT;
    rd = instr.i.rd;
    break;

  case LB:
    funit = U_MEM;
    rd = instr.i.rd;
    break;

  case L_D:
    funit = U_MEM;
    rd = instr.i.rd + 32;
    break;

  case S_D:
    funit = U_MEM;
    rd = 0;
    break;

  case FADD:
  case FSUB:
    funit = U_FPADD;
    rd = instr.r.rd + 32;
    break;

  case FMUL:
    funit = U_FPMUL;
    rd = instr.r.rd + 32;
    break;
  }

  if(current->funit_state[funit].op == NOP && (rd == 0 || current->reg_status[rd] == U_NONE)){
    next->funit_state[funit].time = current->funit_state[funit].cycles;
    next->funit_state[funit].op = instr.j.op;
    if(rd) next->reg_status[rd] = funit;
  } else {
    next->pc = current->pc;
    return;
  }

  switch(instr.j.op){
  case SYSCALL:
    if(current->reg_status[2] != U_NONE || current->reg_status[4] != U_NONE
       || current->reg_status[5] != U_NONE){
      next->pc = current->pc;
      return;
    }
    next->funit_state[funit].rd = current->reg[2];
    next->funit_state[funit].rs = current->reg[4];
    next->funit_state[funit].rs_r = TRUE;
    next->funit_state[funit].rt = current->reg[5];
    next->funit_state[funit].rt_r = TRUE;
    break;

  case B:
    next->funit_state[funit].rd = instr.j.offset;
    next->funit_state[funit].rs = 0;
    next->funit_state[funit].rs_r = TRUE;
    next->funit_state[funit].rt = 0;
    next->funit_state[funit].rt_r = TRUE;
    break;

  case BEQZ:
    next->funit_state[funit].rd = instr.i.offset;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = 0;
    next->funit_state[funit].rt_r = TRUE;
    break;

  case BGE:
    next->funit_state[funit].rd = instr.i.offset;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.i.rd;
    next->funit_state[funit].rt_r = FALSE;
    break;

  case BNE:
    next->funit_state[funit].rd = instr.i.offset;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.i.rd;
    next->funit_state[funit].rt_r = FALSE;
    break;

  case ADD:
  case SUB:
    next->funit_state[funit].rd = instr.r.rd;
    next->funit_state[funit].rs = instr.r.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.r.rt;
    next->funit_state[funit].rt_r = FALSE;
    break;

  case ADDI:
  case SUBI:
    next->funit_state[funit].rd = instr.i.rd;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.i.offset;
    next->funit_state[funit].rt_r = TRUE;
    break;

  case LA:
  case LI:
    next->funit_state[funit].rd = instr.i.rd;
    next->funit_state[funit].rs = 0;
    next->funit_state[funit].rs_r = TRUE;
    next->funit_state[funit].rt = instr.i.offset;
    next->funit_state[funit].rt_r = TRUE;
    break;

  case LB:
    next->funit_state[funit].rd = instr.i.rd;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.i.offset;
    next->funit_state[funit].rt_r = TRUE;
    break;

  case FADD:
  case FSUB:
  case FMUL:
    next->funit_state[funit].rd = instr.r.rd + 32;
    next->funit_state[funit].rs = instr.r.rs + 32;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.r.rt + 32;
    next->funit_state[funit].rt_r = FALSE;
    break;

  case L_D:
    next->funit_state[funit].rd = instr.i.rd + 32;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.i.offset;
    next->funit_state[funit].rt_r = TRUE;
    break;

  case S_D:
    next->funit_state[funit].rd = instr.i.offset;
    next->funit_state[funit].rs = instr.i.rs;
    next->funit_state[funit].rs_r = FALSE;
    next->funit_state[funit].rt = instr.i.rd + 32;
    next->funit_state[funit].rt_r = FALSE;
    break;
  }
}

void exec_read(struct exec_state_t* current, struct exec_state_t* next){
  int i;
  for(i=0;i<NUM_UNITS;i++){
    if(current->funit_state[i].op != NOP){
      if(current->funit_state[i].rs_r == FALSE){
	if(current->reg_status[current->funit_state[i].rs] == U_NONE
	   || current->reg_status[current->funit_state[i].rs] == i){
	  next->funit_state[i].rs = current->reg[current->funit_state[i].rs];
	  next->funit_state[i].rs_r = TRUE;
	}
      }
      
      if(current->funit_state[i].rt_r == FALSE){
	if(current->reg_status[current->funit_state[i].rt] == U_NONE
	   || current->reg_status[current->funit_state[i].rs] == i){
	  next->funit_state[i].rt = current->reg[current->funit_state[i].rt];
	  next->funit_state[i].rt_r = TRUE;
	}
      }
    }
  }
}

void exec_units(struct exec_state_t* current, struct exec_state_t* next){
  int i;
  for(i=0;i<NUM_UNITS;i++){
    if(current->funit_state[i].time == 0) continue;
    if(current->funit_state[i].rs_r == FALSE) continue;
    if(current->funit_state[i].rt_r == FALSE) continue;
    
    next->funit_state[i].time = current->funit_state[i].time - 1;
    if(next->funit_state[i].time > 0) continue;
    
    switch(current->funit_state[i].op){
    case SYSCALL:
      switch(current->funit_state[i].rd){
      case 1:
        printf("%d\n",current->funit_state[i].rs);
        break;
      case 4:
        fputs((char*)mem_translate_addr(current->funit_state[i].rs), stdout);
        break;
      case 8:
        fgets((char*)mem_translate_addr(current->funit_state[i].rs), current->funit_state[i].rt, stdin);
        break;
      case 10:
        next->running = 0;
        break;
      }
      break;

    case B:
      next->pc += current->funit_state[i].rd<<2;
      break;

    case BEQZ:
      if(current->funit_state[i].rs == 0){
	next->pc += current->funit_state[i].rd<<2;
      }
      break;

    case BNE:
      if(current->funit_state[i].rs != current->funit_state[i].rt){
	next->pc += current->funit_state[i].rd<<2;
      }
      break;

    case BGE:
      if((int32_t)current->funit_state[i].rs >= (int32_t)current->funit_state[i].rt){
	next->pc += current->funit_state[i].rd<<2;
      }
      break;

    case ADD:
    case ADDI:
      next->funit_state[i].rd = current->funit_state[i].rs + current->funit_state[i].rt;
      break;

    case SUB:
    case SUBI:
      next->funit_state[i].rd = current->funit_state[i].rs - current->funit_state[i].rt;
      break;

    case LA:
      next->funit_state[i].rd = current->data + current->funit_state[i].rt;
      break;

    case LB:
      next->funit_state[i].rd = mem_read8(current->funit_state[i].rs + current->funit_state[i].rt);
      break;

    case LI:
      next->funit_state[i].rd = current->funit_state[i].rt;
      break;

    case L_D:
      next->funit_state[i].rd = mem_read32(current->funit_state[i].rs + current->funit_state[i].rt);
      break;

    case S_D:
      mem_write32(current->funit_state[i].rs + current->funit_state[i].rd, current->funit_state[i].rt);
      break;

    case FADD:
      FLOAT(next->funit_state[i].rd) = FLOAT(current->funit_state[i].rs) + FLOAT(current->funit_state[i].rt);
      break;

    case FSUB:
      FLOAT(next->funit_state[i].rd) = FLOAT(current->funit_state[i].rs) - FLOAT(current->funit_state[i].rt);
      break;

    case FMUL:
      FLOAT(next->funit_state[i].rd) = FLOAT(current->funit_state[i].rs) * FLOAT(current->funit_state[i].rt);
      break;
    }
  }
}

void exec_write(struct exec_state_t* current, struct exec_state_t* next){
  int i, rd;
  for(i=0;i<NUM_UNITS;i++){
    if(current->funit_state[i].time == 0 && current->funit_state[i].op != NOP){
      //Replace this with something not stupid (we need more data in funit_state?)
      for(rd=0;rd<64;rd++) if(current->reg_status[rd] == i) break;
      
      next->funit_state[i].op = NOP;
      if(rd > 0 && rd < 64){
	next->reg[rd] = current->funit_state[i].rd;
	next->reg_status[rd] = U_NONE;
      }
    }
  }
}
