#include "cpu/exec.h"
#include "cc.h"


make_EHelper(test) {
  //TODO

  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO
  rtl_and(&t1, &id_dest->val, &id_src->val); //目的操作数与源操作数相与
  operand_write(id_dest, &t1); //写入目的操作数

  rtl_update_ZFSF(&t0, id_dest->width); //更新ZFSF位
  t0 = 0;
  rtl_set_OF(&t0); //设置OF位为0
  rtl_set_CF(&t0); //设置CF位为0
  print_asm_template2(and);
}
/*CF和OF位都是0，目标操作数等于src和dest的异或*/
make_EHelper(xor) {
  // TODO();
  //dest的值和src的值异或之后赋给dest？
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  // CF = OF = 0

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  print_asm_template2(xor);
}

make_EHelper(or) {
  TODO();

  print_asm_template2(or);
}

make_EHelper(sar) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decinfo.opcode & 0xf;

  rtl_setcc(&s0, cc);
  operand_write(id_dest, &s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  TODO();

  print_asm_template1(not);
}
