#include "asm.h"
#include "asm_impl.h"
#include "list.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_LINE_LENGTH 256

int asm_init(){return 0;}
int asm_cleanup(){return 0;}

struct asm_binary* asm_parse_file(const char* file){
  int i, j, toklen, argc;
  char buf[MAX_LINE_LENGTH], argv[MAX_ARGC][MAX_TOKEN_LEN];
  char* token, operator;

  FILE* fp = fopen(file,"r");
  //TODO: Check for NULL

  uint32_t loc = 0, text_segment, data_segment;
  struct asm_label label;
  struct list* label_list = create_list(16,sizeof(struct asm_label));
  struct asm_entry entry, *entryptr;
  struct list* entry_list = create_list(256,sizeof(struct asm_entry));
  struct asm_instr* instr;
  struct asm_binary* bin;

  while(!feof(fp)){
    fgets(buf,MAX_LINE_LENGTH,fp);
    
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
      if(token[toklen-2] == ':'){
	token[toklen-2] = 0;
	label.loc = loc;
	strcpy(label.name,token);
	list_add(label_list,&label);
      }else if(token[0]='.'){
	token++;
	for(i=0;i<toklen;i++){
	  token[i]=tolower(token[i]);
	}
	if(strcmp("text",token) == 0){
	  text_segment = loc;
	}else if(strcmp("data",token) == 0){
	  data_segment = loc;
	}else if(strcmp("word",token) == 0){
	  entry.type = DATA;
	  entry.loc = loc;
	  entry.size = 2;
	  entry.entry.data = atoi(strtok(NULL," \t\n\v\f\r"));
	  list_add(entry_list,&entry);
	  loc += 2;
	}
      }else{
	entry.type = INSTR;
	entry.loc = loc;
	operator = token;
	argc = 0;
	while((token = strtok(NULL,", \t\n\v\f\r")) != NULL){
	  assert(argc <= MAX_ARGC);
	  strcpy(argv[argc++],token);
	}
	instr = asm_decode_instr(token,argc,argv);
	memcpy(&entry.entry.instr,instr,sizeof(struct asm_instr));
	free(instr);
	entry.size = 1 + (argc<<2); //1byte for opcode + 4bytes per argument
	list_add(entry_list,&entry);
	loc += entry.size;
      }
      token = strtok(NULL," \t\n\v\f\r");
    }
  }

  fclose(fp);

  for(i=0;i<entry_list->size;i++){
    entryptr = (struct asm_entry*)list_get(entry_list,i);
    if(entryptr->type == INSTR){
      instr = &entryptr->entry.instr;
      for(j=0;j<instr->argc;i++){
	if(instr->argv[j].type == REFERENCE){
	  //TODO: Resolve reference
	}
      }
    }
  }

  bin = _create_binary();
  bin->size = loc;
  bin->text_segment = text_segment;
  bin->data_segment = data_segment;
  bin->binary = (uint8_t*)malloc(bin->size);

  for(i=0,loc=0;i<entry_list->size;i++){
    entryptr = (struct asm_entry*)list_get(entry_list,i);
    assert(entryptr->loc == loc);
    memcpy(bin->binary+loc,entryptr->entry,entryptr->size);
    loc += entryptr->size;
  }
  assert(loc == bin->size);

  list_delete(entry_list);
  list_delete(label_list);

  return bin;
}

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
