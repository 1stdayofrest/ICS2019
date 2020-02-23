#ifndef __X86_RTL_H__
#define __X86_RTL_H__

#include "rtl/rtl.h"

/* RTL pseudo instructions */

static inline void rtl_lr(rtlreg_t *dest, int r, int width) {
  switch (width) {
  case 4:
    rtl_mv(dest, &reg_l(r));
    return; //系统中断操作
  case 1:
    rtl_host_lm(dest, &reg_b(r), 1);
    return;
  case 2:
    rtl_host_lm(dest, &reg_w(r), 2);
    return;
  default:
    assert(0);
  }
}

static inline void rtl_sr(int r, const rtlreg_t *src1, int width) {
  switch (width) {
  case 4:
    rtl_mv(&reg_l(r), src1);
    return;
  case 1:
    rtl_host_sm(&reg_b(r), src1, 1);
    return;
  case 2:
    rtl_host_sm(&reg_w(r), src1, 2);
    return;
  default:
    assert(0);
  }
}

static inline void rtl_push(const rtlreg_t *src1) {
  // esp <- esp - 4
  rtl_subi(&cpu.esp, &cpu.esp, 4);
  // M[esp] <- src1
  rtl_sm(&cpu.esp, src1, 4);
  // TODO();每次push操作都会使esp向低地值移动4位
}

static inline void rtl_pop(rtlreg_t *dest) {
  // dest <- M[esp]
  rtl_lm(dest, &cpu.esp, 4);
  // esp <- esp + 4
  //  TODO();
  rtl_addi(&cpu.esp, &cpu.esp, 4);
}

static inline void rtl_is_sub_overflow(rtlreg_t *dest, const rtlreg_t *res,
                                       const rtlreg_t *src1,
                                       const rtlreg_t *src2, int width) {
  // dest <- is_overflow(src1 - src2)
  //TODO
}

static inline void rtl_is_sub_carry(rtlreg_t *dest, const rtlreg_t *res,
                                    const rtlreg_t *src1) {
  // dest <- is_carry(src1 - src2)
  //TODO
}

static inline void rtl_is_add_overflow(rtlreg_t *dest, const rtlreg_t *res,
                                       const rtlreg_t *src1,
                                       const rtlreg_t *src2, int width) {
  // dest <- is_overflow(src1 + src2)
  //TODO
}

static inline void rtl_is_add_carry(rtlreg_t *dest, const rtlreg_t *res,
                                    const rtlreg_t *src1) {
  // dest <- is_carry(src1 + src2)
  //  TODO
  *dest = *src1 + *res;
}
//对eflags寄存器的set，get操作，简单赋值操作？？？
#define make_rtl_setget_eflags(f)                                              \
  static inline void concat(rtl_set_, f)(const rtlreg_t *src) {                \
    cpu.eflags.f = *src & 0x1;                                                       \
  }                                                                            \
  static inline void concat(rtl_get_, f)(rtlreg_t * dest) {                    \
    *dest = cpu.eflags.f;                                                      \
  }
make_rtl_setget_eflags(CF)
make_rtl_setget_eflags(OF)
make_rtl_setget_eflags(ZF)
make_rtl_setget_eflags(SF)
//简单的判断之后就传到rtl_set_ZF函数里面设置给ZF相应的值

static inline void rtl_update_ZF(const rtlreg_t *result, int width) {
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  // TODO
  //int zf = 0;
  if (width == 1) {
    t0 = (*result & 0x000000ff) | 0;
  }
  else if (width == 2) {
    t0 = (*result & 0x0000ffff) | 0;
  }
  else if (width == 4) {
    t0 = (*result & 0xffffffff) | 0;
  }
  //cpu.eflags.ZF = (t0 == 0) ? 1 : 0;
  rtl_set_ZF(&t0);
}
/* 此时我们就可以判断SF位没有写对，因为我们传入了一个负数，
 * SF位应该是1，但是SF位没变。然后检查rtl_SF函数，
 * 发现2.1写的这个函数有问题，取到的符号位是错的，
 * 更改这个函数，通过右移相应位数来找到符号位。*/
static inline void rtl_update_SF(const rtlreg_t *result, int width) {
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  // TODO
  rtl_msb(&t0, result, width);
  rtlreg_t is_sign = t0 != 0;
  rtl_set_SF(&is_sign);
  //t0 = result[width * 8 - 1]; //找到符号位
  //rtl_set_SF(&t0);            //设置符号位
}

static inline void rtl_update_ZFSF(const rtlreg_t *result, int width) {
  rtl_update_ZF(result, width);
  rtl_update_SF(result, width);
}

#endif
