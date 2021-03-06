#include "cpu/exec.h"
#include "monitor/monitor.h"
// nop指令？？？
make_EHelper(nop) {
  //空指令,什么都不做。
  print_asm("nop");
}
/* 错误代码
 * 打印错误信息*/
make_EHelper(inv) {
  /* invalid opcode */

  uint32_t temp[2];
  *pc = cpu.pc;
  temp[0] = instr_fetch(pc, 4);
  temp[1] = instr_fetch(pc, 4);

  uint8_t *p = (void *)temp;
  printf("invalid opcode(PC = 0x%08x): %02x %02x %02x %02x %02x %02x %02x %02x "
         "...\n\n",
         cpu.pc, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

  display_inv_msg(cpu.pc);

  rtl_exit(NEMU_ABORT, cpu.pc, -1);

  print_asm("invalid opcode");
}

make_EHelper(nemu_trap) {
  difftest_skip_ref();

  rtl_exit(NEMU_END, cpu.pc, cpu.eax);

  print_asm("nemu trap");
  return;
}
