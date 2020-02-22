#include "cpu/exec.h"
#include "all-instr.h"

static inline void set_width(int width) {
  if (width == 0) {
    width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  }
  decinfo.src.width = decinfo.dest.width = decinfo.src2.width = width;
}

static make_EHelper(2byte_esc);

#define make_group(name, item0, item1, item2, item3, item4, item5, item6,      \
                   item7)                                                      \
  static OpcodeEntry concat(opcode_table_, name)[8] = {                        \
      /* 0x00 */ item0, item1, item2, item3,                                   \
      /* 0x04 */ item4, item5, item6, item7};                                  \
  static make_EHelper(name) {                                                  \
    idex(pc, &concat(opcode_table_, name)[decinfo.isa.ext_opcode]);            \
  }
/* 另一种方法是使用ModR/M字节中的扩展opcode域来对opcode的长度进行扩充.
 * 有些时候, 读入一个字节也还不能完全确定具体的指令形式,
 * 这时候需要读入紧跟在opcode后面的ModR/M字节,
 * 把其中的reg/opcode域当做opcode的一部分来解释,
 * 才能决定具体的指令形式.
 * x86把这些指令划分成不同的指令组(instruction group),
 * 在同一个指令组中的指令需要通过
 * ModR/M字节中的扩展opcode域来区分.*/
/* 0x80, 0x81, 0x83 */
make_group(gp1, EX(add), EX(or), EX(adc), EX(sbb), EX(and), EX(sub), EX(xor),
           EX(cmp))
    /* 0xc0, 0xc1, 0xd0, 0xd1, 0xd2, 0xd3 */
    make_group(gp2, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY)

    /* 0xf6, 0xf7 */
    make_group(gp3, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY)

    /* 0xfe */
    make_group(gp4, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY)

    /* 0xff */
make_group(gp5,
           EX(inc), EX(dec), EX(call_rm), EX(call),
           EX(jmp_rm), EMPTY, EX(push), EMPTY)
    /* 0x0f 0x01*/
    make_group(gp7, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY)

    /* TODO: Add more instructions!!! */
    /* 译码查找表,
     * 这一张表通过操作码opcode来索引,
     * 它记录了每一个opcode对应指令
     * 的译码辅助函数, 执行辅助函数, 以及操作数宽度.
     * 有了这些信息, 我们就可以得知指令的具体操作了,
     * 例如对两个寄存器进行加法操作
     * */
    static OpcodeEntry opcode_table[512] = {
        /* 0x00 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x04 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x08 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x0c */ EMPTY, EMPTY, EMPTY, EX(2byte_esc),
        /* 0x10 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x14 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x18 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x1c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x20 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x24 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x28 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x2c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x30 */ IDEXW(G2E, xor, 1), IDEX(G2E, xor), IDEXW(E2G, xor, 1),
        IDEX(E2G, xor),
        /* 0x34 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x38 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x3c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x40 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x44 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x48 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x4c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x50 */ IDEX(r, push), IDEX(r, push), IDEX(r, push), IDEX(r, push),
        /* 0x54 */ IDEX(r, push), IDEX(r, push), IDEX(r, push), IDEX(r, push),
        /* 0x58 */ IDEX(r, pop), IDEX(r, pop), IDEX(r, pop), IDEX(r, pop),
        /* 0x5c */ IDEX(r, pop), IDEX(r, pop), IDEX(r, pop), IDEX(r, pop),
        /* 0x60 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x64 */ EMPTY, EMPTY, EX(operand_size), EMPTY,
        /* 0x68 */ IDEX(push_SI, push), EMPTY, EMPTY, EMPTY,
        /* 0x6c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x70 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x74 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x78 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x7c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x80 */ IDEXW(I2E, gp1, 1), IDEX(I2E, gp1), EMPTY, IDEX(SI2E, gp1),
        /* 0x84 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x88 */ IDEXW(mov_G2E, mov, 1), IDEX(mov_G2E, mov),
        IDEXW(mov_E2G, mov, 1), IDEX(mov_E2G, mov),
        /* 0x8c */ EMPTY, IDEX(lea_M2G, lea), EMPTY, EMPTY,
        /* 0x90 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x94 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x98 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x9c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xa0 */ IDEXW(O2a, mov, 1), IDEX(O2a, mov), IDEXW(a2O, mov, 1),
        IDEX(a2O, mov),
        /* 0xa4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xa8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xac */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xb0 */ IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
        IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
        /* 0xb4 */ IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
        IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
        /* 0xb8 */ IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov),
        IDEX(mov_I2r, mov),
        /* 0xbc */ IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov),
        IDEX(mov_I2r, mov),
        /* 0xc0 */ IDEXW(gp2_Ib2E, gp2, 1), IDEX(gp2_Ib2E, gp2), EMPTY, EX(ret),
        /* 0xc4 */ EMPTY, EMPTY, IDEXW(mov_I2E, mov, 1), IDEX(mov_I2E, mov),
        /* 0xc8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xcc */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xd0 */ IDEXW(gp2_1_E, gp2, 1), IDEX(gp2_1_E, gp2),
        IDEXW(gp2_cl2E, gp2, 1), IDEX(gp2_cl2E, gp2),
        /* 0xd4 */ EMPTY, EMPTY, EX(nemu_trap), EMPTY,
        /* 0xd8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xdc */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xe0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xe4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xe8 */ IDEXW(J, call, 0), EMPTY, EMPTY, EMPTY,
        /* 0xec */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xf0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xf4 */ EMPTY, EMPTY, IDEXW(E, gp3, 1), IDEX(E, gp3),
        /* 0xf8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xfc */ EMPTY, EMPTY, IDEXW(E, gp4, 1), IDEX(E, gp5),

        /*2 byte_opcode_table */

        /* 0x00 */ EMPTY, IDEX(gp7_E, gp7), EMPTY, EMPTY,
        /* 0x04 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x08 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x0c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x10 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x14 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x18 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x1c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x20 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x24 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x28 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x2c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x30 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x34 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x38 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x3c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x40 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x44 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x48 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x4c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x50 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x54 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x58 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x5c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x60 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x64 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x68 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x6c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x70 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x74 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x78 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x7c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x80 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x84 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x88 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x8c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x90 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x94 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x98 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0x9c */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xa0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xa4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xa8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xac */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xb0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xb4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xb8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xbc */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xc0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xc4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xc8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xcc */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xd0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xd4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xd8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xdc */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xe0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xe4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xe8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xec */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xf0 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xf4 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xf8 */ EMPTY, EMPTY, EMPTY, EMPTY,
        /* 0xfc */ EMPTY, EMPTY, EMPTY, EMPTY};

static make_EHelper(2byte_esc) {
  uint32_t opcode = instr_fetch(pc, 1) | 0x100;
  decinfo.opcode = opcode;
  set_width(opcode_table[opcode].width);
  idex(pc, &opcode_table[opcode]);
}

/* decinfo.seq_pc的地址将被作为参数送进isa_exec()函数。
 * isa_exec()显然是一个用于屏蔽ISA差异的API:
 * 不同ISA的指令行为天然不同
 * isa_exec()做的第一件事情就是取指令.
 * 在NEMU中, 有一个函数instr_fetch()
 * (在nemu/include/cpu/exec.h中定义)专门负责取指令的工作.
 * 阅读这个函数的代码, 你就会发现取指操作的本质:
 * 不过就是一次内存的访问而已.
 * */
void isa_exec(vaddr_t *pc) {
  // 取指(instruction fetch, IF)
  uint32_t opcode = instr_fetch(pc, 1);
  decinfo.opcode = opcode;
  /* 操作数宽度的处理
   * mips32和riscv32的访存指令会有不同操作数宽度的版本,
   * 包括32位, 16位和8位,
   * 因此我们还需要把宽度信息记录到decinfo中提供后续过程使用.
   * 对于x86来说就更复杂了,
   * 大部分指令都有不同操作数宽度的版本,
   * 因此x86的操作数宽度信息记录会更复杂:
   * 首先通过set_width()函数记录译码查找表中记录的操作数宽度;
   * 若这一操作数宽度结果为0, 表示仅仅根据操作码来判断,
   * 操作数宽度还不能确定, 可能是16位或者32位,
   * 需要通过decinfo.isa.is_operand_size_16成员变量来决定.
   * 这其实实现了"操作数宽度前缀"的相关功能
   * */
  set_width(opcode_table[opcode].width);
  /* 译码和执行
   * 这个从译码查找表中取得的元素将会被作为参数,
   * 调用idex()函数(在nemu/include/cpu/exec.h中定义).
   * 顾名思义, 这个函数就是用来进行译码和执行的.
   * */
  idex(pc, &opcode_table[opcode]);
}
