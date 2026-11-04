// Xmamba as a Spike extension library.
//
//   spike --isa=rv32im_zicsr --extlib=build/libxmamba.so --extension=xmamba prog.elf
//
// Every custom-0 instruction is decoded here and executed by xm_exec() from
// xmamba_ref.c, the single source of truth for the semantics. The unit's state
// (accumulator, scan memory) lives in the per-hart extension object. Undefined
// encodings and out-of-range indices raise an illegal-instruction trap.
#include <riscv/extension.h>
#include <riscv/processor.h>
#include <riscv/trap.h>
#include <riscv/disasm.h>
#include <memory>
#include <string>
#include <vector>

#include "xmamba_ref.h"

namespace {

class xmamba_t : public extension_t
{
 public:
  std::unique_ptr<xm_unit_t> unit{new xm_unit_t};
  xmamba_t() { xm_reset(unit.get()); }
  const char *name() const override { return "xmamba"; }
  std::vector<insn_desc_t> get_instructions(const processor_t &) override;
  std::vector<disasm_insn_t *> get_disasms(const processor_t *) override;
  void reset(processor_t &) override { xm_reset(unit.get()); }
};

reg_t xm_insn(processor_t *p, insn_t insn, reg_t pc)
{
  auto *x = static_cast<xmamba_t *>(p->get_extension("xmamba"));
  state_t *s = p->get_state();
  uint32_t rs1 = (uint32_t)s->XPR[insn.rs1()];
  uint32_t rs2 = (uint32_t)s->XPR[insn.rs2()];
  uint32_t rd = 0;
  if (xm_exec(x->unit.get(), (unsigned)insn.funct3(), (unsigned)insn.funct7(), rs1, rs2, &rd))
    throw trap_illegal_instruction(insn.bits());
  reg_t v = (reg_t)(sreg_t)(int32_t)rd;           // sign-extend like any RV32 result
  s->log_reg_write[insn.rd() << 4] = {v, 0};
  s->XPR.write(insn.rd(), v);
  return pc + 4;
}

std::vector<insn_desc_t> xmamba_t::get_instructions(const processor_t &)
{
  // One decoder entry for the whole custom-0 opcode; xm_exec() sorts out funct3/funct7.
  insn_desc_t d = {XM_OPCODE, 0x7f, xm_insn, xm_insn, xm_insn, xm_insn,
                   xm_insn, xm_insn, xm_insn, xm_insn};
  return {d};
}

// ------------------------------------------------------------ disassembly

const char *const reg_names[32] = {
  "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

struct reg_arg_t : arg_t {
  int field;  // 0 = rd, 1 = rs1, 2 = rs2
  explicit reg_arg_t(int f) : field(f) {}
  std::string to_string(insn_t i) const override
  {
    return reg_names[field == 0 ? i.rd() : field == 1 ? i.rs1() : i.rs2()];
  }
};

const reg_arg_t arg_rd(0), arg_rs1(1), arg_rs2(2);

std::vector<disasm_insn_t *> xmamba_t::get_disasms(const processor_t *)
{
  struct op { const char *name; unsigned f3, f7; std::vector<const arg_t *> args; };
  const std::vector<op> ops = {
    {"mb.mac4",     XM_F3_MAC,  XM_F7_MAC4,     {&arg_rs1, &arg_rs2}},
    {"mb.racc",     XM_F3_MAC,  XM_F7_RACC,     {&arg_rd, &arg_rs1}},
    {"mb.exp",      XM_F3_NL,   XM_F7_EXP,      {&arg_rd, &arg_rs1}},
    {"mb.silu",     XM_F3_NL,   XM_F7_SILU,     {&arg_rd, &arg_rs1}},
    {"mb.softplus", XM_F3_NL,   XM_F7_SOFTPLUS, {&arg_rd, &arg_rs1}},
    {"mb.rst",      XM_F3_SCAN, XM_F7_RST,      {}},
    {"mb.layer",    XM_F3_SCAN, XM_F7_LAYER,    {&arg_rs1}},
    {"mb.lda",      XM_F3_SCAN, XM_F7_LDA,      {&arg_rs1, &arg_rs2}},
    {"mb.ldbc",     XM_F3_SCAN, XM_F7_LDBC,     {&arg_rs1, &arg_rs2}},
    {"mb.scan",     XM_F3_SCAN, XM_F7_SCAN,     {&arg_rd, &arg_rs1, &arg_rs2}},
    {"mb.rdh",      XM_F3_SCAN, XM_F7_RDH,      {&arg_rd, &arg_rs1}},
  };
  std::vector<disasm_insn_t *> v;
  for (const auto &o : ops) {
    uint32_t match = XM_OPCODE | (o.f3 << 12) | (o.f7 << 25);
    v.push_back(new disasm_insn_t(o.name, match, 0xFE00707F, o.args));
  }
  return v;
}

}  // namespace

REGISTER_EXTENSION(xmamba, []() { return new xmamba_t; })
