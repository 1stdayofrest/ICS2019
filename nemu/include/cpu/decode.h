#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include "common.h"
/* 我们还是不知道操作对象(例如具体要对哪两个寄存器进行加法操作呢).
 * 为了解决这个问题, 框架代码需要进行进一步的译码工作.
 * 具体地, idex()将会根据从译码查找表中取得的内容,
 * 进一步调用相应的译码辅助函数(decode helper function).
 * 译码辅助函数统一通过宏
 * make_DHelper(在nemu/include/cpu/decode.h中定义)来定义:
 * 每个译码辅助函数负责进行一种类型的操作数译码,
 * 把指令中的操作数信息分别记录在
 * 全局译码信息decinfo的src成员, src2成员和dest成员中,
 * 它们分别代表两个源操作数和一个目的操作数.
 * nemu/include/cpu/decode.h中定义了
 * 三个宏id_src, id_src2和id_dest, 用于方便地访问它们.*/
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
  uint32_t type;//操作数类型
  int width;    //操作数宽度
  union {
    uint32_t reg; //寄存器操作数
    rtlreg_t addr;//操作数地址
    uint32_t imm; //立即数操作数
    int32_t simm; //无符号立即操作数
  };
  rtlreg_t val;   //RTL寄存器
  char str[OP_STR_SIZE];//操作数指令
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
