#pragma once

struct asm_binary { 
  uint32_t size;
  uint32_t text_segment;
  uint32_t data_segment;

  uint8_t* binary;
};

int asm_init();
int asm_cleanup();

struct asm_binary* asm_parse(int lines, char** text);

uint32_t asm_decode(char* instr, int argc, char** argv);
