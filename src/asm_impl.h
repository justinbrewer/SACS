#pragma once

#include "asm.h"

#include <stdint.h>

typedef enum { ADDRESS, REFERENCE } asm_arg_type;

struct asm_arg {
  asm_arg_type type;
  union data {
    uint32_t value;
    char* reference;
  };
};

struct asm_instr {
  uint8_t opcode;
  uint8_t argc;
  struct asm_arg* argv;
};

struct asm_binary* _create_binary();
int _delete_binary(struct asm_binary* bin);

struct asm_instr* _create_instr();
int _delete_instr(struct asm_instr* instr);

struct asm_arg* _create_arg(uint8_t max_argc);
int _delete_arg(struct asm_arg* arg);

struct asm_instr* asm_decode_instr(char* instr, int argc, char** argv);
