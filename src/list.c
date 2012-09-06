#include "list.h"

#include <stdlib.h>
#include <string.h>

struct list* list_create(uint32_t size, size_t entry_size){
  struct list* l = (struct list*)malloc(sizeof(struct list));
  l->size = size;
  l->ptr = 0;
  l->entry_size = entry_size;
  l->data = malloc(size*entry_size);
}

int list_delete(struct list* l){
  free(l->data);
  free(l);
}

uint32_t list_add(struct list* l, void* data){
  if(l->size == l->ptr){
    uint32_t new_size = l->size*1.5;
    void* new_data = malloc(new_size*l->entry_size);
    memcpy(new_data,l->data,l->size*l->entry_size);
    free(l->data);
    l->size = new_size;
    l->data = new_data;
  }
  memcpy(l->data+(l->ptr*l->entry_size),data,entry_size);
  return l->ptr++;
}

void* list_get(struct list* l, uint32_t index){
  return l->data+(index*l->entry_size);
}
