#ifndef H_LUAVM
#define H_LUAVM

#ifdef __linux__
#include "eelphant.h"
#else
#include <eelphant.h>
#endif /* __linux__ */
#include "lopcodes.h"

typedef struct luabc_header {
  char signature[4]; //0x1B4C7561
  uint8_t ver; //0x51
  uint8_t format_ver; //0
  uint8_t endianness; //1 (lil)
  uint8_t intsize; //4
  uint8_t sizesize; //4
  uint8_t instrsize; //4
  uint8_t luansize; //8
  uint8_t integral; //0
} lheader_t;

typedef struct luavm_const{
  uint8_t type;
  uint64_t data; //contains value, unless type equals 0, then it's nil, or 4, then it's a pointer to string
} luavm_const;

typedef struct luavm_state {
  ep_window* window;
  lheader_t* file;
  uint32_t stack[MAXSTACK];
  uint32_t ip;
  uint32_t code_n, code_ptr, const_n, const_ptr, fprot_n, fprot_ptr;
  luavm_const constants[200];
  uint32_t registers[200];
  struct {
    uint32_t name;
    uint32_t data;
  } globals[200];
} luavm_state;

typedef struct luavm_func {
  void* ptrblock;

} luavm_func;

#endif
