#include "mem.h"

#include <stdlib.h>
#include <string.h>

#define ROOT_SIZE 32
#define MINIMUM_SIZE 8

typedef enum { FREE=0, SPLIT, FULL, LOCKED, STACK } NodeState;

struct MemoryNode {
  NodeState state;
  uint32_t size;
  uint32_t loc;

  struct MemoryNode* parent;
  struct MemoryNode* left;
  struct MemoryNode* right;

  uint8_t* mem;
};

typedef enum { INSUFFICIENT_SPACE=1 } AllocError;

struct MemoryNode* _create_node(uint32_t size, uint32_t loc, struct MemoryNode* parent);
int _delete_node(struct MemoryNode* node);
struct MemoryNode* _get_node(uint32_t loc);
uint32_t _dyn_alloc(struct MemoryNode* node, uint32_t size);
int _collapse_node(struct MemoryNode* node);

struct MemoryNode* root;

int mem_init(void){
  root = _create_node(ROOT_SIZE,0,0);
  root->state = SPLIT;

  root->left = _create_node(ROOT_SIZE-1,0,root);
  root->left->state = SPLIT;
  root->left->left = _create_node(ROOT_SIZE-2,0,root->left);
  root->left->left->state = LOCKED;

  root->right = _create_node(ROOT_SIZE-1,1<<(ROOT_SIZE-1),root);
  root->right->state = SPLIT;
  root->right->right = _create_node(ROOT_SIZE-2,3<<(ROOT_SIZE-2),root->right);
  root->right->right->state = STACK;
  return 0;
}

int mem_cleanup(void){
  _delete_node(root);
  root = 0;
  return 0;
}

uint32_t mem_dynamic_alloc(uint32_t size){
  uint32_t bit_size = size&(size-1) == 0 ? -1 : 0;
  for(; size != 0; bit_size++, size >>= 1);
  if(bit_size == 0){
    return 0;
  }
  return _dyn_alloc(root,bit_size);
}

int mem_free(uint32_t loc){
  struct MemoryNode* node = _get_node(loc);
  if(node->loc != loc){
    return -1;
  }
  if(node->state == LOCKED || node->state == STACK){
    return -2;
  }
  node->state = FREE;
  return _collapse_node(node->parent);
}

void* mem_translate_addr(uint32_t loc){
  struct MemoryNode* node = _get_node(loc);
  if(node != 0 && node->state == FULL){
    uint32_t offset = loc - node->loc;
    return node->mem + offset;
  }else{
    return 0;
  }
}

uint8_t mem_read8(uint32_t loc){
  return *((uint8_t*)mem_translate_addr(loc));
}

uint16_t mem_read16(uint32_t loc){
  return *((uint16_t*)mem_translate_addr(loc));
}

uint32_t mem_read32(uint32_t loc){
  return *((uint32_t*)mem_translate_addr(loc));
}

uint8_t mem_write8(uint32_t loc, uint8_t data){
  return *((uint8_t*)mem_translate_addr(loc)) = data;
}

uint16_t mem_write16(uint32_t loc, uint16_t data){
  return *((uint16_t*)mem_translate_addr(loc)) = data;
}

uint32_t mem_write32(uint32_t loc, uint32_t data){
  return *((uint32_t*)mem_translate_addr(loc)) = data;
}

struct MemoryNode* _create_node(uint32_t size, uint32_t loc, struct MemoryNode* parent){
  struct MemoryNode* node = (struct MemoryNode*)malloc(sizeof(struct MemoryNode));
  memset(node,0,sizeof(struct MemoryNode));
  node->size = size;
  node->loc = loc;
  node->parent = parent;
  return node;
}

int _delete_node(struct MemoryNode* node){
  if(node->left != 0){
    _delete_node(node->left);
  }
  if(node->right != 0){
    _delete_node(node->right);
  }
  if(node->mem != 0){
    free(node->mem);
  }
  free(node);
}

struct MemoryNode* __get_node(struct MemoryNode* node, uint32_t loc){
  switch(node->state){
  case FREE:
    return node;
  case SPLIT:
    if(loc < node->loc+(1<<(node->size-1))){
      if(node->left != 0){
	return __get_node(node->left,loc);
      }else{
	return 0;//FIXME
      }
    }else{
      if(node->right != 0){
	return __get_node(node->right,loc);
      }else{
	return 0;//FIXME
      }
    }
  case FULL:
    return node;
  }
}

struct MemoryNode* _get_node(uint32_t loc){
  return __get_node(root,loc);
}

uint32_t _dyn_alloc(struct MemoryNode* node, uint32_t size){
  uint32_t res;
  switch(node->state){
  case FREE:
    if(node->size == size || node->size == MINIMUM_SIZE){
      node->state = FULL;
      node->mem = malloc(1 << size);
      return node->loc;
    }else{
      node->state = SPLIT;
      node->left = _create_node(node->size-1,node->loc,node);
      return _dyn_alloc(node->left,size);
    }

  case SPLIT:
    if(node->left == 0){
      node->left = _create_node(node->size-1,node->loc,node);
      return _dyn_alloc(node->left,size);
    }else{
      res = _dyn_alloc(node->left,size);
      if(res == INSUFFICIENT_SPACE){
	if(node->right == 0){
	  node->right = _create_node(node->size-1,node->loc+(1<<(node->size-1)),node); //TESTME
	  return _dyn_alloc(node->right,size);
	}else{
	  return _dyn_alloc(node->right,size);
	}      
      }else{
	return res;
      }
    }

  case LOCKED:
  case STACK:
  case FULL:
    return INSUFFICIENT_SPACE;
  }
  return 0;
}

int _collapse_node(struct MemoryNode* node){
  if(node->left != 0 && node->left->state == FREE){
    _delete_node(node->left);
    node->left = 0;
  }
  if(node->right != 0 && node->right->state == FREE){
    _delete_node(node->right);
    node->right = 0;
  }
  if(node->left == 0 && node->right == 0){
    node->state = FREE;
    return _collapse_node(node->parent);
  }
  return 0;
}
