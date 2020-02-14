#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include "common.h"

#define make_DHelper(name) void concat(decode_, name) (vaddr_t *pc)
/* typedef void (*DHelper) (vaddr_t *)
 * 写成void (*DHelper)，说明 DHelper 是一个指针
 * DHelper 是指向一个返回值为空，参数为（vaddr_t类型的指针）的函数的指针
 * 在加上typedef 之后 DHelper就不是指针了，他是一种类型
 * 这种类型可以定义一种指向返回值为空，参数为（vaddr_t类型的指针）的函数的指针
 * */
typedef void (*DHelper) (vaddr_t *);

#define OP_STR_SIZE 40
enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

typedef struct {
  uint32_t type;
  int width;
  union {
    uint32_t reg;
    rtlreg_t addr;
    uint32_t imm;
    int32_t simm;
  };
  rtlreg_t val;
  char str[OP_STR_SIZE];
} Operand;

#include "isa/decode.h"

typedef struct {
  uint32_t opcode; //
  uint32_t width;  //
  vaddr_t seq_pc;  // sequential pc
  bool is_jmp;     //
  vaddr_t jmp_pc;  //
  Operand src, dest, src2;
  /* decinfo结构在nemu/src/cpu/cpu.c中定义,
   * 它用于记录一些全局译码信息供后续使用,
   * 包括操作数的类型, 宽度, 值等信息. 还有一些信息是ISA相关的,
   * NEMU用一个结构类型struct ISADecodeInfo来对这些信息进行抽象,
   * 具体的定义在nemu/src/isa/$ISA/include/isa/decode.h中
   * 相当于继承关系 ISADecodeInfo 生 DecodeInfo
   * */
  struct ISADecodeInfo isa; //
} DecodeInfo;

void operand_write(Operand *, rtlreg_t *);

/* shared by all helper functions */
extern DecodeInfo decinfo;

#define id_src (&decinfo.src)
#define id_src2 (&decinfo.src2)
#define id_dest (&decinfo.dest)

#ifdef DEBUG
#define print_Dop(...) snprintf(__VA_ARGS__)
#else
#define print_Dop(...)
#endif

#endif
