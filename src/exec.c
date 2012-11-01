#include "exec.h"
#include "instr.h"
#include "mem.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { ALU_ADD=0x00, ALU_SUB=0x01, ALU__SYSCALL=0xFF } exec_alu_op_t;
typedef enum { MEM_NOP=0x00, MEM_RB=0x01, MEM_WRITE=0x02 } exec_mem_op_t;

struct exec_pipe_ifid_t {
  union gpr_instr_t ir;
};

struct exec_pipe_idex_t {
  exec_alu_op_t alu_op;
  uint32_t alu_in1;
  uint32_t alu_in2;

  exec_mem_op_t mem_op;
  uint32_t mem_val;

  uint32_t rs;
  uint32_t rt;
  uint32_t rd;
};

struct exec_pipe_exmem_t {
  uint32_t alu_out;

  exec_mem_op_t mem_op;
  uint32_t mem_val;

  uint32_t rd;
};

struct exec_pipe_memwb_t {
  uint32_t alu_out;

  uint32_t rd;
};

struct exec_state_t {
  uint32_t running;
  uint32_t pc;
  uint32_t text;
  uint32_t data;
  uint32_t reg[32];
  uint32_t stall;
  struct exec_pipe_ifid_t if_id;
  struct exec_pipe_idex_t id_ex;
  struct exec_pipe_exmem_t ex_mem;
  struct exec_pipe_memwb_t mem_wb;
  struct exec_stats_t* stats;
};

void exec_pipe_if(struct exec_state_t* state);
void exec_pipe_id(struct exec_state_t* state);
void exec_pipe_ex(struct exec_state_t* state);
void exec_pipe_mem(struct exec_state_t* state);
void exec_pipe_wb(struct exec_state_t* state);

struct exec_stats_t* exec_run(uint32_t start, uint32_t text, uint32_t data){
  struct exec_state_t state = {0};
  state.running = 1;
  state.pc = start;
  state.text = text;
  state.data = data;

  struct exec_stats_t* stats = (struct exec_stats_t*)malloc(sizeof(struct exec_stats_t));
  memset(stats,0,sizeof(struct exec_stats_t));
  state.stats = stats;

  while(state.running){
    exec_pipe_wb(&state);
    exec_pipe_mem(&state);
    exec_pipe_ex(&state);
    exec_pipe_id(&state);
    exec_pipe_if(&state);

    stats->c++;
  }

  return stats;
}

void exec_pipe_if(struct exec_state_t* state){
  struct exec_pipe_ifid_t* out = &state->if_id;

  if(state->stall == 0){
    out->ir.u = mem_read32(state->pc);
    state->pc += 4;
    state->stats->ic++;
  } else {
    state->stall--;
  }
}

#define ALU(op,a,b)				\
  out->alu_op = op;				\
  out->alu_in1 = a;				\
  out->alu_in2 = b;

#define MEM(op, val)			\
  out->mem_op = op;			\
  out->mem_val = val;

#define REG(_rs,_rt,_rd)				\
  out->rs = _rs;					\
  out->rt = _rt;					\
  out->rd = _rd;

#define CHECK_RAW(r) (r && (state->ex_mem.rd == r || state->mem_wb.rd == r))

#define STALL					\
  state->stall++;				\
  ALU(ALU_ADD,0,0);				\
  MEM(MEM_NOP,0);				\
  REG(0,0,0);					\
  break;

void exec_pipe_id(struct exec_state_t* state){
  struct exec_pipe_ifid_t* in = &state->if_id;
  struct exec_pipe_idex_t* out = &state->id_ex;

  switch(in->ir.j.op){
  case NOP:
    ALU(ALU_ADD,0,0);
    MEM(MEM_NOP,0);
    REG(0,0,0);
    break;

  case SYSCALL: //A bit ugly
    if(CHECK_RAW(2) || CHECK_RAW(4) || CHECK_RAW(5)){ STALL; }
    ALU(ALU__SYSCALL, state->reg[2], state->reg[5]);
    MEM(MEM_NOP, state->reg[4]);
    REG(0,0,2); //FIXME
    break;

  case LA:
    ALU(ALU_ADD, state->data, in->ir.i.offset);
    MEM(MEM_NOP,0);
    REG(0,0,in->ir.i.rd);
    break;

  case LB:
    if(CHECK_RAW(in->ir.i.rs)){ STALL; }
    ALU(ALU_ADD, state->reg[in->ir.i.rs], in->ir.i.offset);
    MEM(MEM_RB,0);
    REG(in->ir.i.rs,0,in->ir.i.rd);
    break;

  case LI:
    ALU(ALU_ADD,in->ir.i.offset,0);
    MEM(MEM_NOP,0);
    REG(0,0,in->ir.i.rd);
    break;

  case B:
    state->stall = 1;
    state->pc += in->ir.j.offset<<2;
    in->ir.u = 0;

    ALU(ALU_ADD,0,0);
    MEM(MEM_NOP,0);
    REG(0,0,0);
    break;

  case BEQZ:
    if(CHECK_RAW(in->ir.i.rs)){ STALL; }
    state->stall = 1;
    if(state->reg[in->ir.i.rs] == 0){
      state->pc += in->ir.i.offset<<2;
    }
    in->ir.u = 0;

    ALU(ALU_ADD,0,0);
    MEM(MEM_NOP,0);
    REG(0,0,0);
    break;

  case BGE:
    if(CHECK_RAW(in->ir.i.rd) || CHECK_RAW(in->ir.i.rs)){ STALL; }
    state->stall = 1;
    if((int32_t)state->reg[in->ir.i.rd] >= (int32_t)state->reg[in->ir.i.rs]){
      state->pc += in->ir.i.offset<<2;
    }
    in->ir.u = 0;

    ALU(ALU_ADD,0,0);
    MEM(MEM_NOP,0);
    REG(0,0,0);
    break;

  case BNE:
    if(CHECK_RAW(in->ir.i.rd) || CHECK_RAW(in->ir.i.rs)){ STALL; }
    state->stall = 1;
    if(state->reg[in->ir.i.rd] != state->reg[in->ir.i.rs]){
      state->pc += in->ir.i.offset<<2;
    }
    in->ir.u = 0;

    ALU(ALU_ADD,0,0);
    MEM(MEM_NOP,0);
    REG(0,0,0);
    break;

  case ADD:
    if(CHECK_RAW(in->ir.r.rs) || CHECK_RAW(in->ir.r.rt)){ STALL; }
    ALU(ALU_ADD, state->reg[in->ir.r.rs], state->reg[in->ir.r.rt]);
    MEM(MEM_NOP,0);
    REG(in->ir.r.rs,in->ir.r.rt,in->ir.r.rd);
    break;

  case ADDI:
    if(CHECK_RAW(in->ir.i.rs)){ STALL; }
    ALU(ALU_ADD, state->reg[in->ir.i.rs], in->ir.i.offset);
    MEM(MEM_NOP,0);
    REG(in->ir.i.rs,0,in->ir.i.rd);
    break;

  case SUB:
    if(CHECK_RAW(in->ir.r.rs) || CHECK_RAW(in->ir.r.rt)){ STALL; }
    ALU(ALU_SUB, state->reg[in->ir.r.rs], state->reg[in->ir.r.rt]);
    MEM(MEM_NOP,0);
    REG(in->ir.r.rs,in->ir.r.rt,in->ir.r.rd);
    break;

  case SUBI:
    if(CHECK_RAW(in->ir.i.rs)){ STALL; }
    ALU(ALU_SUB, state->reg[in->ir.i.rs], in->ir.i.offset);
    MEM(MEM_NOP,0);
    REG(in->ir.i.rs,0,in->ir.i.rd);
    break;
  }
}

void exec_pipe_ex(struct exec_state_t* state){
  struct exec_pipe_idex_t* in = &state->id_ex;
  struct exec_pipe_exmem_t* out = &state->ex_mem;

  out->mem_op = in->mem_op;
  out->mem_val = in->mem_val;
  out->rd = in->rd;

  switch(in->alu_op){
  case ALU_ADD:
    out->alu_out = in->alu_in1 + in->alu_in2;
    break;

  case ALU_SUB:
    out->alu_out= in->alu_in1 - in->alu_in2;
    break;

  case ALU__SYSCALL:
    switch(in->alu_in1){
    case 1:
      printf("%d\n",in->mem_val);
      break;
    case 4:
      fputs((char*)mem_translate_addr(in->mem_val), stdout);
      break;
    case 8:
      fgets((char*)mem_translate_addr(in->mem_val), in->alu_in2, stdin);
      break;
    case 10:
      state->running = 0;
      break;
    }
  }
}

void exec_pipe_mem(struct exec_state_t* state){
  struct exec_pipe_exmem_t* in = &state->ex_mem;
  struct exec_pipe_memwb_t* out = &state->mem_wb;

  out->rd = in->rd;
  out->alu_out = in->alu_out;

  switch(in->mem_op){
  case MEM_NOP:
    break;

  case MEM_RB:
    out->alu_out = mem_read8(in->alu_out);
    break;
  }
}

void exec_pipe_wb(struct exec_state_t* state){
  struct exec_pipe_memwb_t* in = &state->mem_wb;

  switch(in->rd){
  case 0:
    break;

  default:
    state->reg[in->rd] = in->alu_out;
    break;
  }
}
