#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if(ref_r->eax != cpu.eax || ref_r->ecx != cpu.ecx || ref_r->edx != cpu.edx ||
     ref_r->ebx != cpu.ebx || ref_r->esp != cpu.esp || ref_r->ebp != cpu.ebp ||
     ref_r->esi != cpu.esi || ref_r->edi != cpu.edi || ref_r->pc != cpu.pc) {
    printf("qemus eax:0x%08x, mine eax:0x%08x @pc:0x%08x\n", ref_r->eax, cpu.eax, cpu.pc);
    printf("qemus ecx:0x%08x, mine ecx:0x%08x @pc:0x%08x\n", ref_r->ecx, cpu.ecx, cpu.pc);
    printf("qemus edx:0x%08x, mine edx:0x%08x @pc:0x%08x\n", ref_r->edx, cpu.edx, cpu.pc);
    printf("qemus ebx:0x%08x, mine ebx:0x%08x @pc:0x%08x\n", ref_r->ebx, cpu.ebx, cpu.pc);
    printf("qemus esp:0x%08x, mine esp:0x%08x @pc:0x%08x\n", ref_r->esp, cpu.esp, cpu.pc);
    printf("qemus ebp:0x%08x, mine ebp:0x%08x @pc:0x%08x\n", ref_r->ebp, cpu.ebp, cpu.pc);
    printf("qemus esi:0x%08x, mine esi:0x%08x @pc:0x%08x\n", ref_r->esi, cpu.esi, cpu.pc);
    printf("qemus edi:0x%08x, mine edi:0x%08x @pc:0x%08x\n", ref_r->edi, cpu.edi, cpu.pc);
    printf("qemus pc :0x%08x, mine pc :0x%08x @pc:0x%08x\n", ref_r->pc, cpu.pc, cpu.pc);
    return true;
  }

  return false;
}

void isa_difftest_attach(void) {
}
