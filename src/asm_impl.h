#pragma once

#include "asm.h"

#include <stdint.h>

#define MAX_TOKEN_LEN  16
#define MAX_ARGC 4
#define MAX_ENTRY_SIZE 1024

typedef enum { VALUE, REFERENCE } asm_arg_type;
typedef enum { INSTR, DATA } asm_entry_type;

struct asm_arg {
  asm_arg_type type;
  union {
    uint32_t value;
    char reference[MAX_TOKEN_LEN];
  };
};

struct asm_instr {
  uint8_t opcode;
  uint8_t argc;
  struct asm_arg argv[MAX_ARGC];
};

struct asm_entry {
  asm_entry_type type;
  uint32_t loc;
  uint32_t size;

  union {
    struct asm_instr instr;
    uint8_t data[MAX_ENTRY_SIZE];
  };
};

struct asm_label {
  uint32_t loc;
  char name[MAX_TOKEN_LEN];
};

struct asm_binary* _create_binary();
int _delete_binary(struct asm_binary* bin);

struct asm_instr* asm_decode_instr(char* operator, int argc, char argv[MAX_ARGC][MAX_TOKEN_LEN]);
uint32_t asm_collapse_instr(struct asm_instr* instr);
