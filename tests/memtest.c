#include "mem.h"
#include <stdio.h>

int main(void){
  printf("Initializing\n");
  mem_init();

  printf("Allocating 1MB\n");
  uint32_t loc = mem_dynamic_alloc(1<<20);
  
  printf("Writing 192342\n");
  mem_write32(loc,192343);

  printf("Reading...\n");
  uint32_t result = mem_read32(loc);
  printf("Read %d\n",result);

  printf("Freeing\n");
  mem_free(loc);

  printf("Cleaning up\n");
  mem_cleanup();

  return 0;
}
