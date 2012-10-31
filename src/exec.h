#pragma once

#include <stdint.h>

struct exec_stats_t {
  uint32_t c;
  uint32_t ic;
};

struct exec_stats_t* exec_run(uint32_t start, uint32_t text, uint32_t data);
