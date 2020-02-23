#include "cpu/exec.h"
#include "cc.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  rtl_j(decinfo.jmp_pc);

  print_asm("jmp %x", decinfo.jmp_pc);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint32_t cc = decinfo.opcode & 0xf;
  rtl_setcc(&t0, cc);//问题很大兄弟
  rtl_li(&t1, 0);//条件转移指令
  rtl_jrelop(RELOP_NE, &t0, &t1, decinfo.jmp_pc);//条件跳转
  //difftest_skip_ref();
  print_asm("j%s %x", get_cc_name(cc), decinfo.jmp_pc);

}

make_EHelper(jmp_rm) {
  rtl_jr(&id_dest->val);

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {//跳转指令
  // the target address is calculated at the decode stage
  //TODO() rtl push操作是什么意思
  rtl_push(&decinfo.seq_pc);
  decinfo.is_jmp = 1;
  print_asm("call %x", decinfo.jmp_pc);
}

make_EHelper(ret) {
  //TODO();
  rtl_pop(&decinfo.jmp_pc);
  rtl_j(decinfo.jmp_pc);
  print_asm("ret");
}

make_EHelper(ret_imm) {
  TODO();

  print_asm("ret %s", id_dest->str);
}

make_EHelper(call_rm) {
  TODO();
  /*rtl_push(&decinfo.seq_pc);
  decinfo.jmp_pc = id_dest->val;
  decinfo.is_jmp = 1;*/

  print_asm("call *%s", id_dest->str);
}
