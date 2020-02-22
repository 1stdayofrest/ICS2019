#ifndef __X86_REG_H__
#define __X86_REG_H__

#include "common.h"

#define PC_START IMAGE_START

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */
/* x86的通用寄存器有eax、ebx、ecx、edx、edi、esi。这些寄存器在大多数指令中是可以
 * 任意选用的，比如movl指令可以把一个立即数传送到eax中，也可传送到ebx中。
 * 但也有一些指令规定只能用其中某个寄存器做某种用途，
 * 例如除法指令idivl要求被除数在eax寄存器中，edx寄存器必须是0，而除数可以在任意寄存器中，
 * 计算结果的商数保存在eax寄存器中（覆盖原来的被除数），余数保存在edx寄存器中。
 * 也就是说，通用寄存器对于某些特殊指令来说也不是通用的。
 * x86的特殊寄存器有ebp、esp、eip、eflags。eip是程序计数器，
 * eflags保存着计算过程中产生的标志位，
 * 其中包括进位标志、溢出标志、零标志和负数标志，
 * 在intel的手册中这几个标志位分别称为CF、OF、ZF、SF。
 * ebp和esp用于维护函数调用的栈帧*/
typedef struct {
  union {
    union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];
    /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
     * in PA2 able to directly access these registers.
     */
    struct {
      /* Do NOT change the order of the GPRs' definitions. */
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
  };
  vaddr_t pc;
  union {
    struct {
      uint32_t CF : 1; // CF占一位
      unsigned : 5;    //之后是5位空域
      uint32_t ZF : 1; // ZF占一位
      uint32_t SF : 1; // SF占一位
      unsigned : 1;    // 1位空域
      uint32_t IF : 1; // IF占一位
      unsigned : 1;    // 1位空域
      uint32_t OF : 1; // OF占一位
      unsigned : 20;   // 20位空域
    };
    uint32_t value; //赋初值要用
  } eflags;         // eflags寄存器,需要设置eflags的初值
} CPU_state;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

static inline const char *reg_name(int index, int width) {
  extern const char *regsl[];
  extern const char *regsw[];
  extern const char *regsb[];
  assert(index >= 0 && index < 8);

  switch (width) {
  case 4:
    return regsl[index];
  case 1:
    return regsb[index];
  case 2:
    return regsw[index];
  default:
    assert(0);
  }
}

#endif
