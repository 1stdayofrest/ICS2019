#ifndef __RTL_RTL_H__
#define __RTL_RTL_H__

#include "nemu.h"
#include "rtl/c_op.h"
#include "rtl/relop.h"
#include "rtl/rtl-wrapper.h"

extern rtlreg_t s0, s1, t0, t1, t2, t3, ir;

void decinfo_set_jmp(bool is_jmp);
bool interpret_relop(uint32_t relop, const rtlreg_t src1, const rtlreg_t src2);

/* RTL basic instructions */

static inline void interpret_rtl_li(rtlreg_t *dest, uint32_t imm) {
  *dest = imm;
}

static inline void interpret_rtl_mv(rtlreg_t *dest, const rtlreg_t *src1) {
  *dest = *src1;
}
//寄存器操作 寄存器和立即数操作
#define make_rtl_arith_logic(name)                                             \
  static inline void concat(interpret_rtl_, name)(                             \
      rtlreg_t * dest, const rtlreg_t *src1, const rtlreg_t *src2) {           \
    *dest = concat(c_, name)(*src1, *src2);                                    \
  }                                                                            \
  /* Actually those of imm version are pseudo rtl instructions,                \
   * but we define them here in the same macro */                              \
  static inline void concat(rtl_, name##i)(rtlreg_t * dest,                    \
                                           const rtlreg_t *src1, int imm) {    \
    rtl_li(&ir, imm);                                                          \
    rtl_##name(dest, src1, &ir);                                               \
  }

make_rtl_arith_logic(add)
make_rtl_arith_logic(sub)
make_rtl_arith_logic(and)
make_rtl_arith_logic(or)
make_rtl_arith_logic(xor)
make_rtl_arith_logic(shl)
make_rtl_arith_logic(shr)
make_rtl_arith_logic(sar)
make_rtl_arith_logic(mul_lo)
make_rtl_arith_logic(mul_hi)
make_rtl_arith_logic(imul_lo)
make_rtl_arith_logic(imul_hi)
make_rtl_arith_logic(div_q)
make_rtl_arith_logic(div_r)
make_rtl_arith_logic(idiv_q)
make_rtl_arith_logic(idiv_r)

static inline void interpret_rtl_div64_q(rtlreg_t *dest,
                                         const rtlreg_t *src1_hi,
                                         const rtlreg_t *src1_lo,
                                         const rtlreg_t *src2) {
  uint64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  uint32_t divisor = (*src2);
  *dest = dividend / divisor;
}

static inline void interpret_rtl_div64_r(rtlreg_t *dest,
                                         const rtlreg_t *src1_hi,
                                         const rtlreg_t *src1_lo,
                                         const rtlreg_t *src2) {
  uint64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  uint32_t divisor = (*src2);
  *dest = dividend % divisor;
}

static inline void interpret_rtl_idiv64_q(rtlreg_t *dest,
                                          const rtlreg_t *src1_hi,
                                          const rtlreg_t *src1_lo,
                                          const rtlreg_t *src2) {
  int64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  int32_t divisor = (*src2);
  *dest = dividend / divisor;
}

static inline void interpret_rtl_idiv64_r(rtlreg_t *dest,
                                          const rtlreg_t *src1_hi,
                                          const rtlreg_t *src1_lo,
                                          const rtlreg_t *src2) {
  int64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  int32_t divisor = (*src2);
  *dest = dividend % divisor;
}

static inline void interpret_rtl_lm(rtlreg_t *dest, const rtlreg_t *addr,
                                    int len) {
  *dest = vaddr_read(*addr, len);
}

static inline void interpret_rtl_sm(const rtlreg_t *addr, const rtlreg_t *src1,
                                    int len) {
  vaddr_write(*addr, *src1, len);
}

static inline void interpret_rtl_host_lm(rtlreg_t *dest, const void *addr,
                                         int len) {
  switch (len) {
  case 4:
    *dest = *(uint32_t *)addr;
    return;
  case 1:
    *dest = *(uint8_t *)addr;
    return;
  case 2:
    *dest = *(uint16_t *)addr;
    return;
  default:
    assert(0);
  }
}

static inline void interpret_rtl_host_sm(void *addr, const rtlreg_t *src1,
                                         int len) {
  switch (len) {
  case 4:
    *(uint32_t *)addr = *src1;
    return;
  case 1:
    *(uint8_t *)addr = *src1;
    return;
  case 2:
    *(uint16_t *)addr = *src1;
    return;
  default:
    assert(0);
  }
}

static inline void interpret_rtl_setrelop(uint32_t relop, rtlreg_t *dest,
                                          const rtlreg_t *src1,
                                          const rtlreg_t *src2) {
  *dest = interpret_relop(relop, *src1, *src2);
}

static inline void interpret_rtl_j(vaddr_t target) {
  cpu.pc = target;
  decinfo_set_jmp(true);
}

static inline void interpret_rtl_jr(rtlreg_t *target) {
  cpu.pc = *target;
  decinfo_set_jmp(true);
}

static inline void interpret_rtl_jrelop(uint32_t relop, const rtlreg_t *src1,
                                        const rtlreg_t *src2, vaddr_t target) {
  bool is_jmp = interpret_relop(relop, *src1, *src2);
  if (is_jmp)
    cpu.pc = target;
  decinfo_set_jmp(is_jmp);
}

void interpret_rtl_exit(int state, vaddr_t halt_pc, uint32_t halt_ret);

/* RTL pseudo instructions */

static inline void rtl_not(rtlreg_t *dest, const rtlreg_t *src1) {
  // dest <- ~src1
  *dest = ~*src1;
}
/*符号位扩展
 * 这个函数写的时候是费了我好大的劲，
 * 想了很多种情况，然后问了助教之后，
 * 不管src是多少位，通通扩展到32位，这样以来就简单一些，
 * 可以通过与一个数相或或者左移右移的方法来实现符号扩展，
 * 这里我选择的是左移右移的方式。
 * 因为寄存器都是无符号整型，所以左移右移没办法移入1，
 * 所以要设置一个带符号整数，通过带符号整数来左移右移，
 * 最后赋给dest
 */
static inline void rtl_sext(rtlreg_t *dest, const rtlreg_t *src1, int width) {
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  // TODO();
  int32_t temp =
      (int32_t)*src1; //设置一个带符号的整型变量将src1，强转成32位带符号操作数
  switch (width) {
  case 1: {//8位1字节
    temp = temp << 24; //左移24位到最高位
    temp = temp >> 24; //右移24位到最低位
  } break;
  case 2: {//16位2字节
    temp = temp << 16;
    temp = temp >> 16;
  } break;
  case 4: {//32位4字节
    break;
  }
  default://没有其他情况
    assert(0);
  }
  *dest = temp;//赋给目的操作数
}
//更新标志符操作
static inline void rtl_setrelopi(uint32_t relop, rtlreg_t *dest,
                                 const rtlreg_t *src1, int imm) {
  rtl_li(&ir, imm);
  rtl_setrelop(relop, dest, src1, &ir);
}
//找符号位，把src1指向数的符号位，赋给dest
static inline void rtl_msb(rtlreg_t *dest, const rtlreg_t *src1, int width) {
  // dest <- src1[width * 8 - 1]
  //TODO
  //*dest = src1[width * 8 - 1];
  *dest = (*src1 >> (width * 8 - 1)) & 0x1;
}

static inline void rtl_mux(rtlreg_t *dest, const rtlreg_t *cond,
                           const rtlreg_t *src1, const rtlreg_t *src2) {
  // dest <- (cond ? src1 : src2)
  //TODO();
  *dest = *cond ? *src1 : *src2;
}

#include "isa/rtl.h"

#endif
