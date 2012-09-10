#include "mem.h"

#include <stdlib.h>
#include <string.h>

#define ROOT_SIZE 32
#define MINIMUM_SIZE 8

typedef enum { FREE=0, SPLIT, FULL, LOCKED } mem_node_state;

struct mem_node {
  mem_node_state state;
  uint32_t size;
  uint32_t loc;

  struct mem_node* parent;
  struct mem_node* left;
  struct mem_node* right;

  uint8_t* mem;
};

typedef enum { INSUFFICIENT_SPACE=1 } mem_error;

struct mem_node* _create_node(uint32_t size, uint32_t loc, struct mem_node* parent);
int _delete_node(struct mem_node* node);
struct mem_node* _get_node(uint32_t loc);
uint32_t _dyn_alloc(struct mem_node* node, uint32_t size, mem_alloc_flags flags);
int _collapse_node(struct mem_node* node);

struct mem_node* root;

/** \brief Initializes memory module

    Initializes root of memory tree and marks the lowest gigabyte as locked.
 */
int mem_init(void){
  root = _create_node(ROOT_SIZE,0,0);
  root->state = SPLIT;

  root->left = _create_node(ROOT_SIZE-1,0,root);
  root->left->state = SPLIT;
  root->left->left = _create_node(ROOT_SIZE-2,0,root->left);
  root->left->left->state = LOCKED;
  return 0;
}

/** \brief Cleans up memory module
    
    Frees entire virtual memory space, as well as any real allocated memory.
 */
int mem_cleanup(void){
  _delete_node(root);
  root = 0;
  return 0;
}

/** \brief Allocates a block of memory

    The size will be rounded up to the next power of two (eg. requesting 
    140 bytes will allocate a block of 256 bytes.)

    If the MEM_FAKE_ALLOC flag is set, the virtual block of memory will
    be created and marked as full, but no real memory will be allocated.
    If the MEM_USE_LOCKED flag is set, it will allocate memory in the
    reserved first gigabyte address space.

    \param size The size requested, in bytes
    \param flags Control flags

    \return Starting address of allocated block
 */
uint32_t mem_dynamic_alloc(uint32_t size, mem_alloc_flags flags){
  uint32_t bit_size = !(size & (size-1)) ? -1 : 0;
  for(; size != 0; bit_size++, size >>= 1);
  if(bit_size == 0){
    return 0;
  }
  return _dyn_alloc(root,bit_size,flags);
}

/** \brief Frees a block of memory
    \param loc The starting address of the block to free
    \return 0 on success, negative on failure
 */
int mem_free(uint32_t loc){
  struct mem_node* node = _get_node(loc);
  if(node->loc != loc){
    return -1;
  }
  if(node->state == LOCKED){
    return -2;
  }
  node->state = FREE;
  return _collapse_node(node->parent);
}

/** \brief Translates virtual address to real address
    \param loc Virtual address to translate
    \return Pointer to real memory
 */
void* mem_translate_addr(uint32_t loc){
  struct mem_node* node = _get_node(loc);
  if(node != 0 && node->state == FULL){
    uint32_t offset = loc - node->loc;
    return node->mem + offset;
  }else{
    return 0;
  }
}

/** \brief Reads a byte from memory
    \param loc Address to read
    \return Value read
 */
uint8_t mem_read8(uint32_t loc){
  return *((uint8_t*)mem_translate_addr(loc));
}

/** \brief Reads a word from memory
    \param loc Address to read
    \return Value read
 */
uint16_t mem_read16(uint32_t loc){
  return *((uint16_t*)mem_translate_addr(loc));
}

/** \brief Reads a doubleword from memory
    \param loc Address to read
    \return Value read
 */
uint32_t mem_read32(uint32_t loc){
  return *((uint32_t*)mem_translate_addr(loc));
}

/** \brief Write a byte to memory
    \param loc Address to write
    \param data Value to write
    \return Value written
 */
uint8_t mem_write8(uint32_t loc, uint8_t data){
  return *((uint8_t*)mem_translate_addr(loc)) = data;
}

/** \brief Write a word to memory
    \param loc Address to write
    \param data Value to write
    \return Value written
 */
uint16_t mem_write16(uint32_t loc, uint16_t data){
  return *((uint16_t*)mem_translate_addr(loc)) = data;
}

/** \brief Write a doubleword to memory
    \param loc Address to write
    \param data Value to write
    \return Value written
 */
uint32_t mem_write32(uint32_t loc, uint32_t data){
  return *((uint32_t*)mem_translate_addr(loc)) = data;
}

struct mem_node* _create_node(uint32_t size, uint32_t loc, struct mem_node* parent){
  struct mem_node* node = (struct mem_node*)malloc(sizeof(struct mem_node));
  memset(node,0,sizeof(struct mem_node));
  node->size = size;
  node->loc = loc;
  node->parent = parent;
  return node;
}

int _delete_node(struct mem_node* node){
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

struct mem_node* __get_node(struct mem_node* node, uint32_t loc){
  switch(node->state){
  case LOCKED:
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
  case FREE:
  case FULL:
    return node;
  }
}

struct mem_node* _get_node(uint32_t loc){
  return __get_node(root,loc);
}

uint32_t _dyn_alloc(struct mem_node* node, uint32_t size, mem_alloc_flags flags){
  uint32_t res;
  switch(node->state){
  case FREE:
    if(node->size == size || node->size == MINIMUM_SIZE){
      node->state = FULL;
      if(!(flags & MEM_FAKE_ALLOC)){
	node->mem = malloc(1 << size);
      }
      return node->loc;
    }else{
      node->state = SPLIT;
      node->left = _create_node(node->size-1,node->loc,node);
      return _dyn_alloc(node->left,size,flags);
    }

  case LOCKED:
    if(!(flags & MEM_USE_LOCKED)){
      return INSUFFICIENT_SPACE;
    }
  case SPLIT:
    if(node->left == 0){
      node->left = _create_node(node->size-1,node->loc,node);
      return _dyn_alloc(node->left,size,flags);
    }else{
      res = _dyn_alloc(node->left,size,flags);
      if(res == INSUFFICIENT_SPACE){
	if(node->right == 0){
	  node->right = _create_node(node->size-1,node->loc+(1<<(node->size-1)),node); //TESTME
	  return _dyn_alloc(node->right,size,flags);
	}else{
	  return _dyn_alloc(node->right,size,flags);
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

int _collapse_node(struct mem_node* node){
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
