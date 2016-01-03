#include <stdio.h>
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>
#include "luavm.h"
#ifdef __linux__ //we don't have these headers on easiOS
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "eelphant.h"
#else /* __linux__ */
#include <eelphant.h>
#endif /* __linux__ */

#define luavm_inf(format, ...) printf("[luaVM INF] " format "\n", ##__VA_ARGS__)
#define luavm_war(format, ...) printf("[luaVM WAR] " format "\n", ##__VA_ARGS__)
#define luavm_err(format, ...) printf("[luaVM ERR] " format "\n", ##__VA_ARGS__)
#define luavm_fat(format, ...) printf("[luaVM FAT] " format "\n", ##__VA_ARGS__)

#ifdef __linux__
extern ep_window w;

void* luacfile;

void luavm_spawn(lheader_t* f);
void luavm_loadfile(const char* fn);

static void signal_catch(int signo)
{
  printf("Signal caught: %s\n", strsignal(signo));
  if(signo == SIGINT || signo == SIGILL)
  {
    printf("exiting...\n");
    if(w.flags) if(w.unload) w.unload(&w);
    if(luacfile) free(luacfile);
    eelphant_destroy_window(&w);
    exit(0);
  }
}

int main(int argc, char** argv)
{
  printf("easiOS luaVM\n");
  if(argc < 2)
  {
    luavm_fat("Usage: %s filename", argv[0]);
    exit(4);
  }
  luavm_inf("setting up signal handler");
  if(signal(SIGINT, signal_catch) == SIG_ERR || signal(SIGILL, signal_catch) == SIG_ERR)
  {
    luavm_fat("error while setting the signal handler");
    return 3;
  }
  luavm_loadfile(argv[1]);
  luavm_spawn(luacfile);
  if(w.load) w.load(&w);
  while(w.flags && !(w.flags >> 2 & 1))
  {
    if(w.update) w.update(100, &w);
  }
  return 0;
}

void luavm_loadfile(const char* fn)
{
  struct stat finfo;
  stat(fn, &finfo);
  luavm_inf("file size: %d bytes", finfo.st_size);
  FILE* f = fopen(fn, "rb");
  if(!f)
  {
    perror("Error opening file");
    exit(1);
  }
  luacfile = malloc(sizeof(char) * finfo.st_size);
  fread(luacfile, finfo.st_size, 1, f);
  fclose(f);
  lheader_t* header = (lheader_t*)luacfile;
  const char* sgn = LUA_SIGNATURE;
  if(memcmp(header->signature, &sgn, 4) == 0)
  {
    luavm_fat("file not lua bytecode (0x%x)", header->signature);
    free(luacfile);
    exit(2);
  }
  if(header->ver != 0x51)
  {
    luavm_fat("file not lua 5.1 bytecode (%d)", header->ver);
    free(luacfile);
    exit(3);
  }
  luavm_inf("file ok");
}

#endif /* __linux__ */

void luavm_exec(luavm_state* state, uint32_t instruction);

#define LUAVM_OPCODE(instruction) instruction & 0x3F
#define LUAVM_A(instruction) instruction >> 6 & 0xFF
#define LUAVM_B(instruction) instruction >> 23
#define LUAVM_C(instruction) instruction >> 18 & 0x3FFF
#define LUAVM_Bx(instruction) instruction >> 14
#define LUAVM_sBx(instruction) LUAVM_Bx(instruction)

void luavm_load()
{

}

void luavm_update(uint64_t dt, ep_window* w)
{
  if(!(w->flags >> 1 & 1)) return;
  luavm_state* s = (luavm_state*)w->userdata[0];
  Instruction* instr = (Instruction*)((uint32_t)s->code_ptr + s->ip);
  for(int i = 0; i < dt && ((w->flags >> 1) & 1); i++)
  {
    if(s->ip + i >= s->code_n)
    {
      luavm_inf("execution finished");
      w->flags &= ~(1 << 1);
      break;
    }
    luavm_exec(s, instr[i]);
  }
  s->ip += dt;
}

void luavm_spawn(lheader_t* f) //spawn a luavm and it's window
{
  #define CHECK(byte, shouldbe, errmsg) if(byte != shouldbe) {luavm_fat(errmsg "(%d)", byte); return;}
  const char* sgn = LUA_SIGNATURE;
  if(memcmp(f->signature, &sgn, 4) == 0)
  {
    luavm_fat("not lua bytecode (sign: %.4s, should be: %.4s)", f->signature, LUA_SIGNATURE);
    return;
  }
  /*if(f->ver != 0x51)
  {
    printf("wrong version (should be 0x51)\n");
    return;
  }*/
  CHECK(f->ver, 0x51, "bad lua version");
  CHECK(f->format_ver, 0, "bad format");
  CHECK(f->endianness, 1, "bad endianness");
  CHECK(f->intsize, 4, "bad int size");
  CHECK(f->sizesize, 4, "bad size_t size");
  CHECK(f->instrsize, 4, "bad instruction size");
  CHECK(f->luansize, 8, "bad lua_Number size");
  CHECK(f->integral, 0, "bad integral flag");
  //printf("top func dump\n");
  void* topfunc = (void*)(((uint32_t)f) + 12);
  uint32_t srcnamel = *(uint32_t*)topfunc;
  //printf("srcname len: %d\n", srcnamel);
  //printf("srcname: \"");
  /*for(int i = 0; i < srcnamel; i++)
  {
    putc(*(char*)(topfunc + 4 + i), stdout);
    printf("%x ", *(char*)(topfunc + 4 + i));
  }
  printf("\"\n");*/

  //--------------------------------------
  ep_window* w = eelphant_create_window();
  if(!w) return;
  strcpy(w->title, "LuaVM");
  w->x = 10;
  w->y = 10;
  w->w = 400;
  w->h = 300;
  w->userdata[0] = (uint32_t)malloc(sizeof(luavm_state));
  if(!w->userdata[0])
  {
    luavm_fat("cannot allocate memory for VM state");
    eelphant_destroy_window(w);
    return;
  }
  luavm_state* vm_state = (luavm_state*)w->userdata[0];
  vm_state->file = f;
  vm_state->ip = 0;
  vm_state->code_ptr = (uint32_t)(topfunc + 4 + srcnamel + 4 + 4 + 4 + 4);
  vm_state->code_n = *(uint32_t*)(vm_state->code_ptr - 4);
  vm_state->const_ptr = vm_state->code_ptr + vm_state->code_n * 4 + 4;
  vm_state->const_n = *(uint32_t*)(vm_state->const_ptr - 4);
  uint8_t* kptr = (uint8_t*)vm_state->const_ptr;
  for(int i = 0; i < vm_state->const_n; i++)
  {
    int len = 0;
    uint64_t debug;
    switch(*kptr)
    {
      case 0:

        luavm_inf("constant #%d: Nil", i);
        vm_state->constants[i].type = 0;
        kptr++;
        break;
      case 1:
        vm_state->constants[i].data = *(uint64_t*)(kptr+1);
        luavm_inf("constant #%d: %f (Boolean)", i, vm_state->constants[i].data);
        kptr += 9;
        break;
      case 3:
        vm_state->constants[i].data = *(uint64_t*)(kptr+1);
        luavm_inf("constant #%d: %f (Number)", i, vm_state->constants[i].data);
        kptr += 9;
        break;
      case 4:
        len = *(uint32_t*)(kptr + 1);
        vm_state->constants[i].data = (uint32_t)kptr + 5;
        luavm_inf("constant #%d: \"%s\" (String)", i, vm_state->constants[i].data);
        kptr += len + 5;
        break;
      default:
        luavm_err("unknown constant #%d with value %d", i, vm_state->constants[i].type);
        break;
    }
  }
  vm_state->fprot_n = *(uint32_t*)(kptr);
  kptr += 4;
  if(vm_state->fprot_n != 0)
  {
    luavm_fat("bytecodes contains user-defined functions: this isn't allowed");
    free((void*)w->userdata[0]);
    eelphant_destroy_window(w);
    return;
  }

  /*for(int i = 0; i < vm_state->fprot_n; i++)
  {
    uint32_t len = *(uint32_t*)kptr;
    kptr += 4 + len;
    uint32_t linedef = *(uint32_t*)kptr;
    kptr += 4;
    uint32_t lastlinedef = *(uint32_t*)kptr;
    printf("Func: Len: %d Linedef: %d Lastlinedef: %d\n", len, linedef, lastlinedef);
  }

  func_skip:*/
  //printf("Code n: %d\t@: 0x%x\nConst n: %d\t@: 0x%x\nFprot n: %d\t @:\n", vm_state->code_n, vm_state->code_ptr, vm_state->const_n, vm_state->const_ptr, vm_state->fprot_n);
  memset(vm_state->stack, 0, MAXSTACK * 4);
  w->flags |= 1 << 1;
  w->update = &luavm_update;
  eelphant_switch_active(w);
}

void luavm_exec(luavm_state* state, uint32_t instruction)
{
  uint32_t opcode = LUAVM_OPCODE(instruction);
  uint8_t a = LUAVM_A(instruction);
  uint16_t b = LUAVM_B(instruction);
  uint16_t c = LUAVM_C(instruction);
  uint32_t bx = LUAVM_Bx(instruction);
  printf("------  %d  -----------\n", instruction);
  switch(opcode)
  {
    //case OP_MOVE:
      //break;
    case OP_LOADK: //load constant into register
      luavm_inf("load constant #%u into register %u", bx - 1, a);
      state->registers[a] = state->constants[bx - 1].data;
      break;
    case OP_SETGLOBAL: //set global value
      luavm_inf("set global #%u to value at register %u", bx - 1, a);
      state->globals[bx - 1].name = bx - 1;
      state->globals[bx - 1].data = state->registers[a];
      break;
    case OP_GETGLOBAL:
      luavm_inf("load global #%u into register %u", bx, a);
      state->registers[a] = state->globals[bx - 1].data;
      break;
    case OP_RETURN:
      luavm_inf("return");
      break;
    case OP_ADD:
      luavm_inf("add value at register %u and %u, store result in register %u", b, c, a);
      state->registers[a] = (uint64_t)((double)state->registers[b] + (double)state->registers[c]);
      printf("\t\tresult: %f\n", state->registers[a]);
      break;
    case OP_CALL:
      luavm_inf("call function at register %d", a);
      if(b == 0) printf("\t\twith an unknown number of parameters\n");
      if(b == 1) printf("\t\twithout parameters\n");
      if(b > 1) printf("\t\twith a number of %d parameters\n", b - 1);

      break;
    default:
      luavm_fat("unimplemented opcode %d", opcode);
      #ifdef __linux__
      raise(SIGILL);
      #else
      eelphant_destroy_window(state->w);
      #endif /* __linux__ */
      break;
  }
}
