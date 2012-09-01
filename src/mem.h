#pragma once
#include <stdint.h>

int mem_init();
int mem_cleanup();

uint32_t mem_dynamic_alloc(uint32_t size);
int mem_free(uint32_t loc);

uint8_t  mem_read8(uint32_t loc);
uint16_t mem_read16(uint32_t loc);
uint32_t mem_read32(uint32_t loc);

int mem_write8(uint32_t loc, uint8_t data);
int mem_write16(uint32_t loc, uint16_t data);
int mem_write32(uint32_t loc, uint32_t data);
