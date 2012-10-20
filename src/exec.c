#include "exec.h"
#include "instr.h"
#include "mem.h"

#include <stdint.h>
#include <stdio.h>

typedef enum { ALU_NOP=0x00, ALU_ADD=0x01, ALU_SUB=0x02, ALU__SYSCALL=0xFF } exec_alu_op_t;
typedef enum { MEM_NOP=0x00, MEM_RB=0x01, MEM_WRITE=0x02 } exec_mem_op_t;

struct exec_pipe_ifid_t {
  union gpr_instr_t ir;
};

struct exec_pipe_idex_t {
  exec_alu_op_t alu_op;
  uint32_t alu_in1;
  uint32_t alu_in2;

  exec_mem_op_t mem_op;
  uint32_t mem_addr;
  uint32_t mem_val;

  uint32_t reg_dest;
  uint32_t reg_val;
};

struct exec_pipe_exmem_t {
  exec_mem_op_t mem_op;
  uint32_t mem_addr;
  uint32_t mem_val;

  uint32_t reg_dest;
  uint32_t reg_val;
};

struct exec_pipe_memwb_t {
  uint32_t reg_dest;
  uint32_t reg_val;
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
};

void exec_pipe_if(struct exec_state_t* state);
void exec_pipe_id(struct exec_state_t* state);
void exec_pipe_ex(struct exec_state_t* state);
void exec_pipe_mem(struct exec_state_t* state);
void exec_pipe_wb(struct exec_state_t* state);

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  struct exec_state_t state = {0};
  state.running = 1;
  state.pc = start;
  state.text = text;
  state.data = data;

  while(state.running){
    exec_pipe_wb(&state);
    exec_pipe_mem(&state);
    exec_pipe_ex(&state);
    exec_pipe_id(&state);
    exec_pipe_if(&state);
  }

  return 0;
}

void exec_pipe_if(struct exec_state_t* state){
  struct exec_pipe_ifid_t* out = &state->if_id;

  if(state->stall == 0){
    out->ir.u = mem_read32(state->pc);
    state->pc += 4;
  } else {
    state->stall--;
    out->ir.u = 0;
  }
}

#define ALU(op,a,b)				\
  out->alu_op = op;				\
  out->alu_in1 = a;				\
  out->alu_in2 = b;

#define MEM(op, addr, val)			\
  out->mem_op = op;				\
  out->mem_addr = addr;				\
  out->mem_val = val;

#define REG(dest, val)				\
  out->reg_dest = dest;				\
  out->reg_val = val;

void exec_pipe_id(struct exec_state_t* state){
  struct exec_pipe_ifid_t* in = &state->if_id;
  struct exec_pipe_idex_t* out = &state->id_ex;

  switch(in->ir.j.op){
  case NOP:
    ALU(ALU_NOP,0,0);
    MEM(MEM_NOP,0,0);
    REG(0,0);
    break;

  case SYSCALL: //A bit ugly
    ALU(ALU__SYSCALL, state->reg[2], state->reg[5]);
    MEM(MEM_NOP, state->reg[4], 0);
    REG(2,0);
    break;

  case LA:
    ALU(ALU_ADD, state->data, in->ir.i.offset);
    MEM(MEM_NOP,0,0);
    REG(in->ir.i.rd,0);
    break;

  case LB:
    ALU(ALU_ADD, state->reg[in->ir.i.rs], in->ir.i.offset);
    MEM(MEM_RB,0,0);
    REG(in->ir.i.rd,0);
    break;

  case LI:
    ALU(ALU_NOP,0,0);
    MEM(MEM_NOP,0,0);
    REG(in->ir.i.rd, in->ir.i.offset);
    break;

  case B:
    state->stall = 1;
    state->pc += in->ir.j.offset<<2;

    ALU(ALU_NOP,0,0);
    MEM(MEM_NOP,0,0);
    REG(0,0);
    break;

  case BEQZ:
    state->stall = 1;
    if(state->reg[in->ir.i.rs] == 0){
      state->pc += in->ir.i.offset<<2;
    }

    ALU(ALU_NOP,0,0);
    MEM(MEM_NOP,0,0);
    REG(0,0);
    break;

  case BGE:
    state->stall = 1;
    if(state->reg[in->ir.i.rd] >= state->reg[in->ir.i.rs]){
      state->pc += in->ir.i.offset<<2;
    }

    ALU(ALU_NOP,0,0);
    MEM(MEM_NOP,0,0);
    REG(0,0);
    break;

  case BNE:
    state->stall = 1;
    if(state->reg[in->ir.i.rd] != state->reg[in->ir.i.rs]){
      state->pc += in->ir.i.offset<<2;
    }

    ALU(ALU_NOP,0,0);
    MEM(MEM_NOP,0,0);
    REG(0,0);
    break;

  case ADDI:
    ALU(ALU_ADD, state->reg[in->ir.i.rs], in->ir.i.offset);
    MEM(MEM_NOP,0,0);
    REG(in->ir.i.rd,0);
    break;

  case SUBI:
    ALU(ALU_SUB, state->reg[in->ir.i.rs], in->ir.i.offset);
    MEM(MEM_NOP,0,0);
    REG(in->ir.i.rd,0);
    break;
  }
}

void exec_pipe_ex(struct exec_state_t* state){
  struct exec_pipe_idex_t* in = &state->id_ex;
  struct exec_pipe_exmem_t* out = &state->ex_mem;

  out->mem_op = in->mem_op;
  out->mem_addr = in->mem_addr;
  out->mem_val = in->mem_val;
  out->reg_dest = in->reg_dest;
  out->reg_val = in->reg_val;

  switch(in->alu_op){
  case ALU_NOP:
    break;

  case ALU_ADD:
    out->mem_addr = out->reg_val = in->alu_in1 + in->alu_in2;
    break;

  case ALU_SUB:
    out->mem_addr = out->reg_val = in->alu_in1 - in->alu_in2;
    break;

  case ALU__SYSCALL:
    switch(in->alu_in1){
    case 4:
      fputs((char*)mem_translate_addr(in->mem_addr), stdout);
      break;
    case 8:
      fgets((char*)mem_translate_addr(in->mem_addr), in->alu_in2, stdin);
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

  out->reg_dest = in->reg_dest;
  out->reg_val = in->reg_val;

  switch(in->mem_op){
  case MEM_NOP:
    break;

  case MEM_RB:
    out->reg_val = mem_read8(in->mem_addr);
    break;
  }
}

void exec_pipe_wb(struct exec_state_t* state){
  struct exec_pipe_memwb_t* in = &state->mem_wb;

  switch(in->reg_dest){
  case 0:
    break;

  default:
    state->reg[in->reg_dest] = in->reg_val;
    break;
  }
}
