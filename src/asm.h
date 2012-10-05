#pragma once

#include <stdint.h>

struct asm_binary { 
  uint32_t size;
  uint32_t text_segment;
  uint32_t data_segment;

  uint8_t* binary;
};

struct asm_binary* asm_parse_file(const char* file);

void asm_free_binary(struct asm_binary* bin);
