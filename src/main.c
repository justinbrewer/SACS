#include "mem.h"

#include <stdio.h>

int main(int argc, char** argv){
  if(argc != 1){
    printf("Usage: sacs <source-file>\n");
    return 0;
  }



  mem_init();

  mem_cleanup();

  return 0;
}
