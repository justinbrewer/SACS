#include "asm.h"
#include "asm_impl.h"
#include "list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_LINE_LENGTH 256

/** \brief Generates a binary from the given source file

    The assembling process consists of three passes. The first pass iterates
    through the source, keeping track of labels and segments, as well as
    translating instructions from plain assembly into an intermediate format
    with unresolved references. The second pass resolves all references. The
    third pass collapses the instructions and data into the final binary format
    readable by the execution module.

    \param file Path to assembly file
    \return Resulting binary
 */
struct asm_binary* asm_parse_file(const char* file){
  int i, j, k, toklen, argc;
  char buf[MAX_LINE_LENGTH], argv[MAX_ARGC][MAX_TOKEN_LEN];
  char* token, *operator;
  uint32_t loc = 0, text_segment, data_segment, label_base = 0;
  struct asm_label label, *labelptr;
  struct list* label_list = list_create(16,sizeof(struct asm_label));
  struct asm_entry entry, *entryptr;
  struct list* entry_list = list_create(256,sizeof(struct asm_entry));
  struct asm_instr* instr;
  struct asm_binary* bin;

  //--------------
  // Pass 1

  FILE* fp = fopen(file,"r");
  assert(fp != NULL);

  while(!feof(fp)){
    if(fgets(buf,MAX_LINE_LENGTH,fp) == NULL) break;
    
    //Strip Comments
    for(i=0;i<MAX_LINE_LENGTH;i++){
      if(buf[i] == '#'){
	buf[i] = 0;
	break;
      }
    }
    
    token = strtok(buf," \t\n\v\f\r");
    while(token != NULL){
      toklen = strlen(token);

      // Labels
      if(token[toklen-1] == ':'){
	token[toklen-1] = 0;
	label.loc = loc - label_base;
	strcpy(label.name,token);
	list_add(label_list,&label);
      }

      // Segments and Data
      else if(token[0] == '.'){
	token++;
	for(i=0;i<toklen;i++) token[i]=tolower(token[i]);

	if(strcmp("text",token) == 0){
	  text_segment = loc;
	  label_base = loc;
	}
	else if(strcmp("data",token) == 0){
	  data_segment = loc;
	  label_base = loc;
	}
	else if(strcmp("word",token) == 0){
	  entry.type = DATA;
	  entry.loc = loc;
	  entry.size = 4;
	  *(uint32_t*)(&entry.data) = atoi(strtok(NULL," \t\n\v\f\r"));
	  list_add(entry_list,&entry);
	  loc += entry.size;
	}
      }

      //Instructions
      else{
	entry.type = INSTR;
	entry.loc = loc;
	entry.size = 4;

	operator = token;
	argc = 0;
	while((token = strtok(NULL,", \t\n\v\f\r")) != NULL){
	  assert(argc <= MAX_ARGC);
	  strcpy(argv[argc++],token);
	}

	instr = asm_decode_instr(operator,argc,argv);
	memcpy(&entry.instr,instr,sizeof(struct asm_instr));
	free(instr);

	list_add(entry_list,&entry);
	loc += entry.size;
      }
      token = strtok(NULL," \t\n\v\f\r");
    }
  }

  fclose(fp);

  //--------------
  // Pass 2

  //Foreach instruction
  for(i=0;i<entry_list->ptr;i++){
    entryptr = (struct asm_entry*)list_get(entry_list,i);
    if(entryptr->type != INSTR) continue;
    instr = &entryptr->instr;

    //Foreach reference argument
    for(j=0;j<instr->argc;j++){
      if(instr->argv[j].type != REFERENCE) continue;

      //Find the corresponding label
      for(k=0;k<label_list->ptr;k++){
	labelptr = (struct asm_label*)list_get(label_list,k);

	if(strcmp(instr->argv[j].reference,labelptr->name) == 0){
	  instr->argv[j].type = VALUE;
	  instr->argv[j].value = labelptr->loc;
	  break;
	}
      }

      //Bail out if the reference was not resolved
      assert(instr->argv[j].type == VALUE);
    }
  }

  //--------------
  // Pass 3

  bin = _create_binary();
  bin->size = loc;
  bin->text_segment = text_segment;
  bin->data_segment = data_segment;
  bin->binary = (uint8_t*)malloc(bin->size);

  uint8_t* ptr = bin->binary;
  for(i=0;i<entry_list->ptr;i++){
    entryptr = (struct asm_entry*)list_get(entry_list,i);

    //Consistency check
    assert(entryptr->loc == ptr - bin->binary);

    switch(entryptr->type){
    case INSTR:
      *(uint32_t*)(ptr) = asm_collapse_instr(&entryptr->instr);
      ptr += 4;
      break;

    case DATA:
      memcpy(ptr,&entryptr->data,entryptr->size);
      ptr += entryptr->size;
      break;
    }
  }

  //Consistency check
  assert(loc == bin->size);

  list_delete(entry_list);
  list_delete(label_list);

  return bin;
}

/** \brief Properly cleans up an asm_binary struct
    \param bin Binary to free
 */
int asm_free_binary(struct asm_binary* bin){
  return _delete_binary(bin);
}

struct asm_binary* _create_binary(){
  struct asm_binary* bin = (struct asm_binary*)malloc(sizeof(struct asm_binary));
  memset(bin,0,sizeof(struct asm_binary));
  return bin;
}

int _delete_binary(struct asm_binary* bin){
  if(bin->binary != 0){
    free(bin->binary);
  }
  free(bin);
}
