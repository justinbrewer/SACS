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

void exec_pipe_if(uint32_t pc, struct exec_pipe_ifid_t& out);
void exec_pipe_id(struct exec_pipe_ifid_t& in, struct exec_pipe_idex_t& out);
void exec_pipe_ex(struct exec_pipe_idex_t& in, struct exec_pipe_exmem_t& out);
void exec_pipe_mem(struct exec_pipe_exmem_t& in, struct exec_pipe_memwb_t& out);
void exec_pipe_wb(struct exec_pipe_memwb_t& in);

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  
}
