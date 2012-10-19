#include "exec.h"
#include "instr.h"

#include <stdint.h>

struct exec_pipe_ifid_t {
  union gpr_instr_t ir;
};

struct exec_pipe_idex_t {
  union gpr_instr_t ir;
  uint32_t alu_in1;
  uint32_t alu_in2;
};

struct exec_pipe_exmem_t {
  union gpr_instr_t ir;
  uint32_t alu_out;
};

struct exec_pipe_memwb_t {
  union gpr_instr_t ir;
  uint32_t alu_out;
};

struct exec_state_t {
  uint32_t running;
  uint32_t pc;
  uint32_t text;
  uint32_t data;
  uint32_t reg[32];
  struct exec_pipe_ifid_t if_id;
  struct exec_pipe_idex_t id_ex;
  struct exec_pipe_exmem_t ex_mem;
  struct exec_pipe_memwb_t mem_wb;
};

void exec_pipe_if(struct exec_state_t& state);
void exec_pipe_id(struct exec_state_t& state);
void exec_pipe_ex(struct exec_state_t& state);
void exec_pipe_mem(struct exec_state_t& state);
void exec_pipe_wb(struct exec_state_t& state);

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  struct exec_state_t state = {0};
  state.running = 1;
  state.pc = start;
  state.text = text;
  state.data = data;

  while(state.running){
    exec_pipe_wb(state);
    exec_pipe_mem(state);
    exec_pipe_ex(state);
    exec_pipe_id(state);
    exec_pipe_if(state);
  }

  return 0;
}

