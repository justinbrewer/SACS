#include "exec.c"

#include <stdint.h>

typedef enum { FALSE=0, TRUE=1 } bool;
typedef enum { U_NONE=0x00, U_INT, U_FPADD, U_FPMULT, U_FPDIV } exec_funit_t;

union exec_fpr_t {
  uint32_t i;
  float f;        //Hopefully 32 bits
};

struct exec_funit_state_t {
  exec_funit_t unit;
  uint32_t rd;
  uint32_t rs;
  uint32_t rt;
  exec_funit_t rs_p;
  exec_funit_t rt_p;
  bool rs_r;
  bool rt_r;
};

struct exec_state_t {
  uint32_t pc;
  uint32_t text;
  uint32_t data;
  uint32_t reg[32];
  union exec_fpr_t fpr[32];

  struct exec_funit_state_t funit_state[4];
  exec_funit_t reg_status[64];

  struct exec_stats_t* stats;
};
