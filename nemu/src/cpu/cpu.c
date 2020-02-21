#include "cpu/exec.h"

CPU_state cpu;
//临时寄存器
rtlreg_t s0, s1, t0, t1, t2, t3, ir;

/* shared by all helper functions */
DecodeInfo decinfo; //全局译码信息

void decinfo_set_jmp(bool is_jmp) { decinfo.is_jmp = is_jmp; }

void isa_exec(vaddr_t *pc);
/* 在exec_once()中,
 * 框架代码首先将当前的PC保存到全局译码信息decinfo的成员seq_pc中.
 * */
vaddr_t exec_once(void) {
  decinfo.seq_pc = cpu.pc;
  /* decinfo.seq_pc的地址将被作为参数送进isa_exec()函数
   * (在nemu/src/isa/$ISA/exec/exec.c中定义)中.
   * isa_exec()显然是一个用于屏蔽ISA差异的API:
   * 不同ISA的指令行为天然不同
   * decinfo.seq_pc的地址 = &decinfo.seq_pc
   * */
  isa_exec(&decinfo.seq_pc);
  update_pc();

  return decinfo.seq_pc;
}
