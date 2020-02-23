#include "rtl/rtl.h"

/* Condition Code */
/*在cc.c文件里面找到了rtl_setcc函数，从注释来看，
 * 让我查询每个eflag来看看是不是满足条件代码。枚举类型不赋初值，
 * 第一个默认为0，往后依次加一。
 * 这就是讲义中说的哪个需要更新eflags的操作。
 * 根据手册勘误来完成相关的操作！*/
void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
/*
  switch (subcode & 0xe) {
  case CC_O://0
    rtl_get_OF(dest);
    break;
  case CC_B://2
    rtl_get_CF(dest); //小于，通过CF来判断不够减
    break;
  case CC_E://4
    rtl_get_ZF(dest);
    break;
  case CC_BE: { //6
    rtl_get_CF(&t0);
    rtl_get_ZF(&t1);
    rtl_or(dest, &t0, &t1); //小于等于，CF和ZF至少一个要等于1才行
  }break;
  case CC_S: //8
    rtl_get_SF(dest);
    break;
  case CC_L: { //12
    rtl_get_SF(&t0);
    rtl_get_OF(&t1);
    rtl_xor(dest, &t1, &t0); //带符号数的小于，SF不能等于OF
  }break;
  case CC_LE: { //14
    rtl_get_ZF(&t0);
    rtl_get_SF(&t1);
    rtl_get_OF(&t2);
    rtl_xor(&t3, &t1, &t2);
    rtl_or(dest, &t0, &t3); //带符号数的小于等于，ZF=1或者SF不等于OF
  }break;
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }
*/
  /*switch (subcode & 0xe) {
  case CC_O:  //0
    *dest = cpu.eflags.OF;
    break;
  case CC_B:  //2
    *dest = cpu.eflags.CF;
    break;
  case CC_E:  //4
    //printf("i am here\n");
    //printf("zf = %d\n", cpu.eflags.ZF);
    *dest = cpu.eflags.ZF;
    break;
  case CC_BE: //6
    //printf("switch = %x\n", subcode&0xe);
    *dest = ((cpu.eflags.CF) || (cpu.eflags.ZF));
    //printf("dest = %d\n", *dest);
    break;
  case CC_S:  //8
    *dest = cpu.eflags.SF;
    break;
  case CC_L:  //12 c
    *dest = (cpu.eflags.SF != cpu.eflags.OF);
    break;
  case CC_LE: //14 e
    *dest = ((cpu.eflags.ZF) || (cpu.eflags.SF != cpu.eflags.OF));
    break;
  default: panic("should not reach here");
  case CC_P: panic("n86 does not have PF");
  }*/
  switch (subcode & 0xe) {
  case CC_O: *dest = cpu.eflags.OF ? 1 : 0; break;
  case CC_B: *dest = cpu.eflags.CF ? 1 : 0; break;
  case CC_E: *dest = cpu.eflags.ZF ? 1 : 0; break;
  case CC_BE: *dest = cpu.eflags.CF | cpu.eflags.ZF ? 1 : 0; break;
  case CC_S: *dest = cpu.eflags.SF ? 1 : 0; break;
  case CC_L: *dest = cpu.eflags.SF != cpu.eflags.OF ? 1 : 0; break;
  case CC_LE: *dest = cpu.eflags.SF != cpu.eflags.OF || cpu.eflags.ZF ? 1 : 0; break;
  default: panic("should not reach here");
  case CC_P: panic("n86 does not have PF");
  }
  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
  assert(*dest == 0 || *dest == 1);
}
