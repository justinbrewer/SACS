#include "mem.h"

#define MINIMUM_SIZE 8

typedef enum { FREE=0, SPLIT, FULL, LOCKED, STACK } NodeState;

typedef struct {
  NodeState state;
  uint32_t size;
  uint32_t loc;

  MemoryNode* parent;
  MemoryNode* left;
  MemoryNode* right;

  uint8_t* mem;
} MemoryNode;

typedef enum { INSUFFICIENT_SPACE=1 } AllocError;

MemoryNode* _create_node(uint32_t size, uint32_t loc, MemoryNode* parent);
int _delete_node(MemoryNode* node);
MemoryNode* _get_node(uint32_t loc);
uint32_t _dyn_alloc(MemoryNode* node, uint32_t size);
int _collapse_node(MemoryNode* node);

MemoryNode* root;

int mem_init(void){
  root = _create_node(32,0);
  root->state = SPLIT;

  root->left = _create_node(31,0,root);
  root->left->state = SPLIT;
  root->left->left = _create_node(30,0,root->left);
  root->left->left->state = LOCKED;

  root->right = _create_node(31,1<<31,root);
  root->right->state = SPLIT;
  root->right->right = _create_node(30,3<<30,root->right);
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
  MemoryNode* node = _get_node(loc);
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
  MemoryNode* node = _get_node(loc);
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
  return *((uint8_t)mem_translate_addr(loc)) = data;
}

int mem_write16(uint32_t loc, uint16_t data){
  return *((uint16_t)mem_translate_addr(loc)) = data;
}

int mem_write32(uint32_t loc, uint32_t data){
  return *((uint32_t)mem_translate_addr(loc)) = data;
}

MemoryNode* _create_node(uint32_t size, uint32_t loc, MemoryNode* parent){
  MemoryNode* node = (MemoryNode*)malloc(sizeof(MemoryNode));
  memset(node,0,sizeof(MemoryNode));
  node->size = size;
  node->loc = loc;
  node->parent = parent;
  return node;
}

int _delete_node(MemoryNode* node){
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

MemoryNode* __get_node(MemoryNode* node, uint32_t loc){
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

MemoryNode* _get_node(uint32_t loc){
  return __get_node(root,loc);
}

uint32_t _dyn_alloc(MemoryNode* node, uint32_t size){
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

  case FULL:
    return INSUFFICIENT_SPACE;
  }
  return 0;
}

int _collapse_node(MemoryNode* node){
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
