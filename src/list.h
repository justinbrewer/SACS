#pragma once

#include <stddef.h>
#include <stdint.h>

struct list {
  uint32_t size;
  uint32_t ptr;
  size_t entry_size;
  void* data;
};

struct list* list_create(uint32_t size, size_t entry_size);
int list_delete(struct list* l);
uint32_t list_add(struct list* l, void* data);
void* list_get(struct list* l, uint32_t index);
