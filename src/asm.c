#include "asm.h"
#include "instr.h"
#include "list.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_LINE_LENGTH 256
#define MAX_TOKEN_LEN  16
#define MAX_ARGC 4
#define MAX_ENTRY_SIZE 1024

typedef enum { VALUE, REFERENCE } asm_arg_type;
typedef enum { INSTR, DATA } asm_entry_type;
typedef enum { TEXT_SEG, DATA_SEG } asm_segment;

struct asm_arg {
  asm_arg_type type;
  union {
    uint32_t value;
    char reference[MAX_TOKEN_LEN];
  };
};

struct asm_instr {
  uint8_t opcode;
  uint8_t argc;
  struct asm_arg argv[MAX_ARGC];
};

struct asm_entry {
  asm_entry_type type;
  uint32_t loc;
  uint32_t size;

  union {
    struct asm_instr instr;
    uint8_t data[MAX_ENTRY_SIZE];
  };
};

struct asm_label {
  asm_segment segment;
  uint32_t loc;
  char name[MAX_TOKEN_LEN];
};

struct asm_instr*  _decode_instr(char* operator, int argc, char argv[MAX_ARGC][MAX_TOKEN_LEN]);
uint32_t           _collapse_instr(struct asm_instr* instr);

struct asm_binary* _create_binary();
void               _delete_binary(struct asm_binary* bin);

uint32_t           _parse_asciiz(char* in, char* out);
uint32_t           _translate_reg_name(char name[MAX_TOKEN_LEN]);
uint32_t           _translate_fpr_name(char name[MAX_TOKEN_LEN]);

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
  uint32_t loc = 0, text_segment=0, data_segment=0, label_base = 0;
  asm_segment current_segment;
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
	label.segment = current_segment;
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
	  current_segment = TEXT_SEG;
	}
	else if(strcmp("data",token) == 0){
	  data_segment = loc;
	  label_base = loc;
	  current_segment = DATA_SEG;
	}
	else if(strcmp("word",token) == 0){
	  entry.type = DATA;
	  entry.loc = loc;
	  entry.size = 4;
	  *(uint32_t*)(&entry.data) = atoi(strtok(NULL," \t\n\v\f\r"));
	  list_add(entry_list,&entry);
	  loc += entry.size;
	}
	else if(strcmp("float",token) == 0){
	  entry.type = DATA;
	  entry.loc = loc;
	  entry.size = 4;
	  *(float*)(&entry.data) = strtod(strtok(NULL," \t\n\v\f\r"),NULL);
	  list_add(entry_list,&entry);
	  loc += entry.size;
	}
	else if(strcmp("asciiz",token) == 0){
	  entry.type = DATA;
	  entry.loc = loc;

	  token = strtok(NULL,"\"");
	  entry.size = _parse_asciiz(token,(char*)&entry.data);

	  list_add(entry_list,&entry);
	  loc += entry.size;
	}
	else if(strcmp("space",token) == 0){
	  entry.type = DATA;
	  entry.loc = loc;
	  entry.size = atoi(strtok(NULL," \t\n\v\f\r"));
	  memset(&entry.data,0,entry.size);
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

	instr = _decode_instr(operator,argc,argv);
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

	  //Text offsets should be relative, data absolute
	  switch(labelptr->segment){
	  case TEXT_SEG:
	    instr->argv[j].value = (labelptr->loc - entryptr->loc - 4) >> 2;
	    break;
	  case DATA_SEG:
	    instr->argv[j].value = labelptr->loc;
	    break;
	  }

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
      *(uint32_t*)(ptr) = _collapse_instr(&entryptr->instr);
      ptr += 4;
      break;

    case DATA:
      memcpy(ptr,&entryptr->data,entryptr->size);
      ptr += entryptr->size;
      break;
    }
  }

  //Consistency check
  assert(bin->size == ptr - bin->binary);

  list_delete(entry_list);
  list_delete(label_list);

  return bin;
}

/** \brief Properly cleans up an asm_binary struct
    \param bin Binary to free
 */
void asm_free_binary(struct asm_binary* bin){
  _delete_binary(bin);
}

#define CHECK_ARGC							\
  if(instr->argc != argc){						\
    printf("Error: %s takes %d arguments, got %d\n",			\
	   operator,instr->argc,argc);					\
    exit(EXIT_FAILURE);							\
  }

#define SPLIT_IMMREG(i,j)			\
  int _k, _len = strlen(argv[i]);		\
  char* _reg;					\
  for(_k=0;_k<_len;_k++){			\
    if(argv[i][_k] == '('){			\
      argv[i][_k] = 0;				\
      reg = (&argv[i][_k])+1;			\
      continue;					\
    }						\
    if(argv[i][_k] == ')'){			\
      argv[i][_k] = 0;				\
      break;					\
    }						\
  }						\
  strcpy(argv[j],reg);				\

#define REGISTER_ARG(i)						\
  instr->argv[i].type = VALUE;					\
  instr->argv[i].value = _translate_reg_name(argv[i]);

#define LABEL_ARG(i)				\
  instr->argv[i].type = REFERENCE;		\
  strcpy(instr->argv[i].reference,argv[i]);

#define IMM_ARG(i)				\
  instr->argv[i].type = VALUE;			\
  instr->argv[i].value = atoi(argv[i]);

#define FLOATREG_ARG(i)					\
  intsr->argv[i].type = VALUE;				\
  instr->argv[i].value = _translate_fpr_name(argv[i]);

struct asm_instr* _decode_instr(char* operator, int argc, char argv[MAX_ARGC][MAX_TOKEN_LEN]){
  struct asm_instr* instr = (struct asm_instr*)malloc(sizeof(struct asm_instr));

  if(strcmp(operator,"syscall") == 0){
    instr->opcode = SYSCALL;
    instr->argc = 0;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"nop") == 0){
    instr->opcode = NOP;
    instr->argc = 0;
    CHECK_ARGC;
  }

  else if(strcmp(operator,"la") == 0){
    instr->opcode = LA;
    instr->argc = 2;
    CHECK_ARGC;

    REGISTER_ARG(0);
    LABEL_ARG(1);
  }

  else if(strcmp(operator,"lb") == 0){
    instr->opcode = LB;
    instr->argc = 3;

    if(argc != 2){
      printf("Error: %s takes %d arguments, got %d\n",
	     operator,2,argc);
      exit(EXIT_FAILURE);
    }

    SPLIT_IMMREG(1,2);

    REGISTER_ARG(0);
    IMM_ARG(1);
    REGISTER_ARG(2);
  }

  else if(strcmp(operator,"li") == 0){
    instr->opcode = LI;
    instr->argc = 2;
    CHECK_ARGC;

    REGISTER_ARG(0);
    IMM_ARG(1);
  }

  else if(strcmp(operator,"b") == 0){
    instr->opcode = B;
    instr->argc = 1;
    CHECK_ARGC;

    LABEL_ARG(0);
  }

  else if(strcmp(operator,"beqz") == 0){
    instr->opcode = BEQZ;
    instr->argc = 2;
    CHECK_ARGC;

    REGISTER_ARG(0);
    LABEL_ARG(1);
  }

  else if(strcmp(operator,"bge") == 0){
    instr->opcode = BGE;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    LABEL_ARG(2);
  }

  else if(strcmp(operator,"bne") == 0){
    instr->opcode = BNE;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    LABEL_ARG(2);
  }

  else if(strcmp(operator,"add") == 0){
    instr->opcode = ADD;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    REGISTER_ARG(2);
  }

  else if(strcmp(operator,"addi") == 0){
    instr->opcode = ADDI;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    IMM_ARG(2);
  }

  else if(strcmp(operator,"sub") == 0){
    instr->opcode = SUB;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    REGISTER_ARG(2);
  }

  else if(strcmp(operator,"subi") == 0){
    instr->opcode = SUBI;
    instr->argc = 3;
    CHECK_ARGC;

    REGISTER_ARG(0);
    REGISTER_ARG(1);
    IMM_ARG(2);
  }

  else {
    printf("Error: Unknown operator \"%s\"\n",operator);
    exit(EXIT_FAILURE);
  }

  return instr;
}

uint32_t _collapse_instr(struct asm_instr* instr){
  union gpr_instr_t res;
  res.u = 0;

  switch(instr->opcode){
  case SYSCALL:
  case NOP:
    res.j.op = instr->opcode;
    break;

  case ADD:
  case SUB:
    res.r.op = instr->opcode;
    res.r.rs = instr->argv[1].value;
    res.r.rt = instr->argv[2].value;
    res.r.rd = instr->argv[0].value;
    res.r.shift = 0;
    res.r.func = 0;
    break;

  case ADDI:
  case BGE:
  case BNE:
  case SUBI:
    res.i.op = instr->opcode;
    res.i.rs = instr->argv[1].value;
    res.i.rd = instr->argv[0].value;
    res.i.offset = instr->argv[2].value;
    break;

  case B:
    res.j.op = instr->opcode;
    res.j.offset = instr->argv[0].value;
    break;

  case BEQZ:
    res.i.op = instr->opcode;
    res.i.rs = instr->argv[0].value;
    res.i.offset = instr->argv[1].value;
    break;

  case LA:
  case LI:
    res.i.op = instr->opcode;
    res.i.rd = instr->argv[0].value;
    res.i.offset = instr->argv[1].value;
    break;

  case LB:
    res.i.op = instr->opcode;
    res.i.rs = instr->argv[2].value;
    res.i.rd = instr->argv[0].value;
    res.i.offset = instr->argv[1].value;
    break;
  }

  return res.u;
}

struct asm_binary* _create_binary(){
  struct asm_binary* bin = (struct asm_binary*)malloc(sizeof(struct asm_binary));
  memset(bin,0,sizeof(struct asm_binary));
  return bin;
}

void _delete_binary(struct asm_binary* bin){
  if(bin->binary != 0){
    free(bin->binary);
  }
  free(bin);
}

uint32_t _parse_asciiz(char* in, char* out){
  char c;
  uint32_t size = 0;

  while((c = *in++)){
    switch(c){
    case '\\': //Special Characters

      switch(*in++){
      case 'n':
	*out++ = '\n';
	break;

      case '\\':
	*out++ = '\\';
	break;
      }

      size++;
      break;

    default:
      *out++ = c;
      size++;
      break;
    }
  }

  *out++ = 0;
  size++;

  return size;
}

uint32_t _translate_reg_name(char name[MAX_TOKEN_LEN]){
  int i;
  static char register_names[32][5] = {"zero","at","v0","v1","a0","a1","a2","a3",
                                       "t0","t1","t2","t3","t4","t5","t6","t7",
                                       "s0","s1","s2","s3","s4","s5","s6","s7",
                                       "t8","t9","k0","k1","gp","sp","fp","ra"};

  if(name[0] != '$'){
    printf("Error: Expected register name, got \"%s\"\n",name);
    exit(EXIT_FAILURE);
  }

  if(isdigit(name[1])){
    return atoi(name+1);
  }

  for(i=0;i<32;i++){
    if(strcmp(name+1,register_names[i]) == 0){
      break;
    }
  }
  if(i < 32){
    return i;
  } 

  printf("Error: Unknown register \"%s\"\n",name);
  exit(EXIT_FAILURE);
}

uint32_t _translate_fpr_name(char name[MAX_TOKEN_LEN]){
  if(name[0] != '$' || name[1] != 'f' || !isdigit(name[2])){
    printf("Error: Expected floating-point register name, got \"%s\"\n",name);
    exit(EXIT_FAILURE);
  }

  return atoi(name+2);
}
