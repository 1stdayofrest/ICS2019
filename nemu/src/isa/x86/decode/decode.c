#include "cpu/exec.h"

/* 译码辅助函数的命名很直观, 直接反映了操作数的类型和数据流向.
 * 以nemu/src/isa/x86/decode/decode.c为例,
 * 其中的译码辅助函数主要以i386手册附录A中的操作数表示记号来命名.
 * 例如I2r表示将立即数移入寄存器,
 * 其中I表示立即数, 2表示英文to,
 * r表示通用寄存器, 更多的记号请参考i386手册.*/

// decode operand helper
/* 我们会发现, 类似寄存器和立即数这些操作数,
 * 其实是非常常见的操作数类型.
 * 为了进一步实现操作数译码和指令译码的解耦,
 * 框架代码对这些操作数的译码进行了抽象封装,
 * 指令译码过程由若干操作数译码辅助函数
 * (operand decode helper function)组成.
 * 操作数译码辅助函数统一通过宏make_DopHelper来定义
 * 将带有一个load_val参数,
 * 用于控制是否需要将该操作数读出到全局译码信息decinfo供后续使用.
 * 例如如果一个内存操作数是源操作数,
 * 就需要将这个操作数从内存中读出来供后续执行阶段来使用;
 * 如果它仅仅是一个目的操作数, 就不需要从内存读出它的值了,
 * 因为执行这条指令并不需要这个值,
 * 而是将新数据写入相应的内存位置.*/
#define make_DopHelper(name) void concat(decode_op_, name) (vaddr_t *pc, Operand *op, bool load_val)

/* Refer to Appendix A in i386 manual for the explanations of these abbreviations */

/* Ib, Iv
 * 对于立即数指令
 * op->imm = instr_fetch(pc, op->width);
 * 将34 12 00 00这个字节序列原封不动地从内存读入imm变量中,
 * 主机的CPU会按照小端方式来解释这一字节序列,
 * 于是会得到0x1234, 符合我们的预期结果.*/
static inline make_DopHelper(I) {
  /* pc here is pointing to the immediate 为什么pc指向立即数 */
  op->type = OP_TYPE_IMM;
  op->imm = instr_fetch(pc, op->width);
  rtl_li(&op->val, op->imm);//立即数读入 rtl 寄存器，用来记录 立即数

  print_Dop(op->str, OP_STR_SIZE, "$0x%x", op->imm);
}

/* I386 manual does not contain this abbreviation, but it is different from
 * the one above from the view of implementation. So we use another helper
 * function to decode it.
 */
/* sign immediate */
static inline make_DopHelper(SI) {
  assert(op->width == 1 || op->width == 4);

  op->type = OP_TYPE_IMM;

  /* TODO: Use instr_fetch() to read `op->width' bytes of memory
   * pointed by 'pc'. Interpret the result as a signed immediate,
   * and assign it to op->simm.
   * 由于CISC指令变长的特性,
   * x86指令长度和指令形式需要一边取指一边译码来确定,
   * 而不像RISC指令集那样可以泾渭分明地处理取指和译码阶段,
   * 因此你会在x86的操作数译码辅助函数中看到
   * instr_fetch()的操作.
   op->simm = ??? 有符号立即数和无符号立即数的区别
   */
  //读取符号操作数，和有符号的一样？？
  if (op->width == 4) {
    op->simm = instr_fetch(pc, op->width);
  }
  else if (op->width == 2) {
    op->simm = (int16_t)((uint16_t)instr_fetch(pc, op->width));
  }
  else {
    op->simm = (int16_t)(int8_t)((uint8_t)instr_fetch(pc, op->width));
  }
  rtl_li(&op->val, op->simm);

  print_Dop(op->str, OP_STR_SIZE, "$0x%x", op->simm);
}

/* I386 manual does not contain this abbreviation.
 * It is convenient to merge them into a single helper function.
 */
/* AL/eAX */
static inline make_DopHelper(a) {
  op->type = OP_TYPE_REG;
  op->reg = R_EAX;
  if (load_val) {
    rtl_lr(&op->val, R_EAX, op->width);
  }

  print_Dop(op->str, OP_STR_SIZE, "%%%s", reg_name(R_EAX, op->width));
}

/* This helper function is use to decode register encoded in the opcode. */
/* XX: AL, AH, BL, BH, CL, CH, DL, DH
 * eXX: eAX, eCX, eDX, eBX, eSP, eBP, eSI, eDI
 */
static inline make_DopHelper(r) {
  op->type = OP_TYPE_REG; //操作 寄存器
  op->reg = decinfo.opcode & 0x7; //判断寄存器
  if (load_val) {//函数中将带有一个load_val参数, 用于控制是否需要将该操作数读出到全局译码信息decinfo供后续使用.
    rtl_lr(&op->val, op->reg, op->width);//load_val是否需要记录
  } //例如如果一个内存操作数是源操作数, 就需要将这个操作数从内存中读出来供后续执行阶段来使用;
  // 如果它仅仅是一个目的操作数, 就不需要从内存读出它的值了, 因为执行这条指令并不需要这个值,
  // 而是将新数据写入相应的内存位置.
  print_Dop(op->str, OP_STR_SIZE, "%%%s", reg_name(op->reg, op->width));
}

/* I386 manual does not contain this abbreviation.
 * We decode everything of modR/M byte by one time.
 */
/* Eb, Ew, Ev
 * Gb, Gv
 * Cd,
 * M
 * Rd
 * Sw
 */
static inline void decode_op_rm(vaddr_t *pc, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  read_ModR_M(pc, rm, load_rm_val, reg, load_reg_val);
}

/* Ob, Ov */
static inline make_DopHelper(O) {
  op->type = OP_TYPE_MEM;
  rtl_li(&op->addr, instr_fetch(pc, 4));
  if (load_val) {
    rtl_lm(&op->val, &op->addr, op->width);
  }

  print_Dop(op->str, OP_STR_SIZE, "0x%x", op->addr);
}

/* Eb <- Gb
 * Ev <- Gv
 */
make_DHelper(G2E) {
  decode_op_rm(pc, id_dest, true, id_src, true);
}

make_DHelper(mov_G2E) {
  decode_op_rm(pc, id_dest, false, id_src, true);
}

/* Gb <- Eb
 * Gv <- Ev
 */
make_DHelper(E2G) {
  decode_op_rm(pc, id_src, true, id_dest, true);
}

make_DHelper(mov_E2G) {
  decode_op_rm(pc, id_src, true, id_dest, false);
}

make_DHelper(lea_M2G) {
  decode_op_rm(pc, id_src, false, id_dest, false);
}

/* AL <- Ib
 * eAX <- Iv
 */
make_DHelper(I2a) {
  decode_op_a(pc, id_dest, true);
  decode_op_I(pc, id_src, true);
}

/* Gv <- EvIb
 * Gv <- EvIv
 * use for imul */
make_DHelper(I_E2G) {
  decode_op_rm(pc, id_src2, true, id_dest, false);
  decode_op_I(pc, id_src, true);
}

/* Eb <- Ib
 * Ev <- Iv
 */
make_DHelper(I2E) {
  decode_op_rm(pc, id_dest, true, NULL, false);
  decode_op_I(pc, id_src, true);
}

make_DHelper(mov_I2E) {
  decode_op_rm(pc, id_dest, false, NULL, false);
  decode_op_I(pc, id_src, true);
}

/* XX <- Ib
 * eXX <- Iv
 */
make_DHelper(I2r) {
  decode_op_r(pc, id_dest, true);
  decode_op_I(pc, id_src, true);
}

make_DHelper(mov_I2r) {
  decode_op_r(pc, id_dest, false);//操作数译码 目标操作数id_dest
  decode_op_I(pc, id_src, true);  //操作数译码 源操作数 id_src
}

/* used by unary operations */
make_DHelper(I) {
  decode_op_I(pc, id_dest, true);
}

make_DHelper(r) {
  decode_op_r(pc, id_dest, true);
}

make_DHelper(E) {
  decode_op_rm(pc, id_dest, true, NULL, false);
}

make_DHelper(setcc_E) {
  decode_op_rm(pc, id_dest, false, NULL, false);
}

make_DHelper(gp7_E) {
  decode_op_rm(pc, id_dest, false, NULL, false);
}

/* used by test in group3 */
make_DHelper(test_I) {
  decode_op_I(pc, id_src, true);
}

make_DHelper(SI2E) {
  assert(id_dest->width == 2 || id_dest->width == 4);
  decode_op_rm(pc, id_dest, true, NULL, false);
  id_src->width = 1;
  decode_op_SI(pc, id_src, true);
  if (id_dest->width == 2) {
    id_src->val &= 0xffff;
  }
}

make_DHelper(SI_E2G) {
  assert(id_dest->width == 2 || id_dest->width == 4);
  decode_op_rm(pc, id_src2, true, id_dest, false);
  id_src->width = 1;
  decode_op_SI(pc, id_src, true);
  if (id_dest->width == 2) {
    id_src->val &= 0xffff;
  }
}

make_DHelper(gp2_1_E) {
  decode_op_rm(pc, id_dest, true, NULL, false);
  id_src->type = OP_TYPE_IMM;
  id_src->imm = 1;
  rtl_li(&id_src->val, 1);

  print_Dop(id_src->str, OP_STR_SIZE, "$1");
}

make_DHelper(gp2_cl2E) {
  decode_op_rm(pc, id_dest, true, NULL, false);
  id_src->type = OP_TYPE_REG;
  id_src->reg = R_CL;
  rtl_lr(&id_src->val, R_CL, 1);

  print_Dop(id_src->str, OP_STR_SIZE, "%%cl");
}

make_DHelper(gp2_Ib2E) {
  decode_op_rm(pc, id_dest, true, NULL, false);
  id_src->width = 1;
  decode_op_I(pc, id_src, true);
}

/* Ev <- GvIb
 * use for shld/shrd */
make_DHelper(Ib_G2E) {
  decode_op_rm(pc, id_dest, true, id_src2, true);
  id_src->width = 1;
  decode_op_I(pc, id_src, true);
}

/* Ev <- GvCL
 * use for shld/shrd */
make_DHelper(cl_G2E) {
  decode_op_rm(pc, id_dest, true, id_src2, true);
  id_src->type = OP_TYPE_REG;
  id_src->reg = R_CL;
  rtl_lr(&id_src->val, R_CL, 1);

  print_Dop(id_src->str, OP_STR_SIZE, "%%cl");
}

make_DHelper(O2a) {
  decode_op_O(pc, id_src, true);
  decode_op_a(pc, id_dest, false);
}

make_DHelper(a2O) {
  decode_op_a(pc, id_src, true);
  decode_op_O(pc, id_dest, false);
}

make_DHelper(J) {//跳转指令
  decode_op_SI(pc, id_dest, false);
  // the target address can be computed in the decode stage
  decinfo.jmp_pc = id_dest->simm + *pc;//为什么是加这个呢
}

make_DHelper(push_SI) {
  decode_op_SI(pc, id_dest, true);
}

make_DHelper(in_I2a) {
  id_src->width = 1;
  decode_op_I(pc, id_src, true);
  decode_op_a(pc, id_dest, false);
}

make_DHelper(in_dx2a) {
  id_src->type = OP_TYPE_REG;
  id_src->reg = R_DX;
  rtl_lr(&id_src->val, R_DX, 2);

  print_Dop(id_src->str, OP_STR_SIZE, "(%%dx)");

  decode_op_a(pc, id_dest, false);
}

make_DHelper(out_a2I) {
  decode_op_a(pc, id_src, true);
  id_dest->width = 1;
  decode_op_I(pc, id_dest, true);
}

make_DHelper(out_a2dx) {
  decode_op_a(pc, id_src, true);

  id_dest->type = OP_TYPE_REG;
  id_dest->reg = R_DX;
  rtl_lr(&id_dest->val, R_DX, 2);

  print_Dop(id_dest->str, OP_STR_SIZE, "(%%dx)");
}

void operand_write(Operand *op, rtlreg_t* src) {
  if (op->type == OP_TYPE_REG) { rtl_sr(op->reg, src, op->width); }
  else if (op->type == OP_TYPE_MEM) { rtl_sm(&op->addr, src, op->width); }
  else { assert(0); }
}
