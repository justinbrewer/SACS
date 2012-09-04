#pragma once

#include "asm.h"

#include <stdint.h>

typedef enum { ADDRESS, REFERENCE } asm_arg_type;
typedef enum { INSTR, DATA } asm_entry_type;

struct asm_arg {
  asm_arg_type type;
  union data {
    uint32_t address;
    char* reference;
  };
};

struct asm_instr {
  uint8_t opcode;
  uint8_t argc;
  struct asm_arg* argv;
};

struct asm_entry {
  asm_entry_type type;
  uint32_t loc;
  uint32_t size;

  union entry {
    asm_instr* instr;
    uint32_t data;
  };
};

struct asm_label {
  uint32_t loc;
  char* name;
};

struct asm_binary* _create_binary();
int _delete_binary(struct asm_binary* bin);

struct asm_instr* _create_instr(uint8_t max_argc);
int _delete_instr(struct asm_instr* instr);

struct asm_arg* _create_arg();
int _delete_arg(struct asm_arg* arg);

struct asm_entry* _create_entry();
int _delete_entry(struct asm_entry* entry);

struct asm_label* _create_label(uint32_t loc, char* name);
int _delete_entry(struct asm_label* label);

struct asm_instr* asm_decode_instr(char* instr, int argc, char** argv);
