#pragma once

#include <stdint.h>

struct asm_binary { 
  uint32_t size;
  uint32_t text_segment;
  uint32_t data_segment;

  uint8_t* binary;
};

int asm_init();
int asm_cleanup();

struct asm_binary* asm_parse_file(char* file);
struct asm_binary* asm_parse(int lines, char** text);

int asm_free_binary(struct asm_binary* bin);
