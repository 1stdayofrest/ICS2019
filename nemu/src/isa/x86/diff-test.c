#include "nemu.h"
#include "monitor/diff-test.h"
/* 把通用寄存器和PC与从QEMU中读出的寄存器的值进行比较. 若对比结果一致, 函数返回true;
 * 如果发现值不一样, 函数返回false, 框架代码会自动停止客户程序的运行*/
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if(ref_r->eax != cpu.eax || ref_r->ecx != cpu.ecx || ref_r->edx != cpu.edx ||
     ref_r->ebx != cpu.ebx || ref_r->esp != cpu.esp || ref_r->ebp != cpu.ebp ||
     ref_r->esi != cpu.esi || ref_r->edi != cpu.edi || ref_r->pc != cpu.pc) {
    printf("qemus eax:0x%08x, mine eax:0x%08x @pc:0x%08x\n", ref_r->eax, cpu.eax, pc);
    printf("qemus ecx:0x%08x, mine ecx:0x%08x @pc:0x%08x\n", ref_r->ecx, cpu.ecx, pc);
    printf("qemus edx:0x%08x, mine edx:0x%08x @pc:0x%08x\n", ref_r->edx, cpu.edx, pc);
    printf("qemus ebx:0x%08x, mine ebx:0x%08x @pc:0x%08x\n", ref_r->ebx, cpu.ebx, pc);
    printf("qemus esp:0x%08x, mine esp:0x%08x @pc:0x%08x\n", ref_r->esp, cpu.esp, pc);
    printf("qemus ebp:0x%08x, mine ebp:0x%08x @pc:0x%08x\n", ref_r->ebp, cpu.ebp, pc);
    printf("qemus esi:0x%08x, mine esi:0x%08x @pc:0x%08x\n", ref_r->esi, cpu.esi, pc);
    printf("qemus edi:0x%08x, mine edi:0x%08x @pc:0x%08x\n", ref_r->edi, cpu.edi, pc);
    printf("qemus pc :0x%08x, mine pc :0x%08x @pc:0x%08x\n", ref_r->pc, cpu.pc, pc);
    return false;
  }

  return true;
}

void isa_difftest_attach(void) {
}
