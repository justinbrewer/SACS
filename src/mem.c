#include "mem.h"

typedef enum { FREE=0, SPLIT, FULL } NodeState;

typedef struct {
  NodeState state;
  uint32_t size;
  uint32_t loc;

  MemoryNode* parent;
  MemoryNode* left;
  MemoryNode* right;

  uint8_t* mem;
} MemoryNode;

MemoryNode* root;

int mem_init(){
  root = (MemoryNode*)malloc(sizeof(MemoryNode));
  memset(root,0,sizeof(MemoryNode));
  root->size = 32;
  return 0;
}

int mem_cleanup(){
  //No cleanup should really be necessary
  return 0;
}
