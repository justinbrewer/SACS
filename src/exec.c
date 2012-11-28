#include "exec.c"

#include <stdint.h>

#define NUM_UNITS 4

typedef enum { FALSE=0, TRUE=1 } bool;
typedef enum { U_NONE=0, U_INT, U_FPADD, U_FPMULT, U_FPDIV } exec_funit_t;
typedef enum { I_FREE=0, I_ISSUE, I_READ, I_EXEC, I_WRITE } exec_instr_status_t;

union exec_fpr_t {
  uint32_t i;
  float f;        //Hopefully 32 bits
};

struct exec_funit_state_t {
  exec_funit_t unit;
  uint32_t cycle_len;

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

  int running;

  uint32_t reg[32];
  union exec_fpr_t fpr[32];

  exec_instr_status_t instr_status[NUM_UNITS];
  struct exec_funit_state_t funit_state[NUM_UNITS];
  exec_funit_t reg_status[64];

  struct exec_stats_t stats;
};

void exec_issue(struct exec_state_t* current, struct exec_state_t* next);
void exec_read(struct exec_state_t* current, struct exec_state_t* next);
void exec_unit(struct exec_state_t* current, struct exec_state_t* next, uint32_t unit);
void exec_write(struct exec_state_t* current, struct exec_state_t* next);

struct exec_stats_t* exec_run(uint32_t start, uint32_t text, uint32_t data){
  struct exec_state_t current = {0}, next;

  current.running = 1;
  current.pc = start;
  current.text = text;
  current.data = data;

  current.funit_state[0].unit = U_INT;
  current.funit_state[0].cycle_len = 1;

  current.funit_state[1].unit = U_FPADD;
  current.funit_state[1].cycle_len = 10;

  current.funit_state[2].unit = U_FPMULT;
  current.funit_state[2].cycle_len = 20;

  current.funit_state[3].unit = U_FPDIV;
  current.funit_state[3].cycle_len = 40;

  memcpy(&next,&current,sizeof(struct exec_state_t));

  while(current.running){
    exec_issue(&current,&next);
    exec_read(&current,&next);
    for(i=0;i<NUM_UNITS;i++) exec_unit(&current,&next,i);
    exec_write(&current,&next);

    memcpy(&current,&next,sizeof(struct exec_state_t));
  }

  struct exec_stats_t* stats = (struct exec_stats_t*)malloc(sizeof(struct exec_stats_t));
  memcpy(stats,&current.stats,sizeof(struct exec_stats_t));
  return stats;
}

void exec_issue(struct exec_state_t* current, struct exec_state_t* next){
  
}

void exec_read(struct exec_state_t* current, struct exec_state_t* next){
  
}

void exec_unit(struct exec_state_t* current, struct exec_state_t* next, uint32_t unit){
  
}

void exec_write(struct exec_state_t* current, struct exec_state_t* next){
  
}
