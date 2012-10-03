#include "gpr/instr.h"

union gpr_ir_t {
  uint32_t u;
  struct gpr_instr_r r;
  struct gpr_instr_i i;
  struct gpr_instr_j j;
};
