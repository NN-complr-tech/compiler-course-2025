#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class SIMDCounterMachinePass : public MachineFunctionPass {
public:
  static char ID;
  SIMDCounterMachinePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override { return "SIMD Instruction Counter"; }

private:
  bool isX86SIMDInstruction(const MachineInstr &MI);
};

char SIMDCounterMachinePass::ID = 0;

bool SIMDCounterMachinePass::runOnMachineFunction(MachineFunction &MF) {
  if (MF.empty())
    return false;

  bool modified = false;

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      if (isX86SIMDInstruction(MI)) {
        MI.setFlag(MachineInstr::MIFlag::NoMerge);
        modified = true;
        errs() << "Found SIMD instruction: " << MI << "\n";
      }
    }
  }

  return modified;
}

bool SIMDCounterMachinePass::isX86SIMDInstruction(const MachineInstr &MI) {
  if (MI.isMetaInstruction() || MI.isLabel() || MI.isDebugInstr()) {
    return false;
  }

  unsigned Opcode = MI.getOpcode();

  if (Opcode >= X86::ADDPSrr && Opcode <= X86::XORPSrr)
    return true;
  if (Opcode >= X86::VADDPSrr && Opcode <= X86::VXORPSrr)
    return true;
  if (Opcode >= X86::VADDPSYrr && Opcode <= X86::VXORPSYrr)
    return true;
  if (Opcode >= X86::ADDPDrr && Opcode <= X86::XORPDrr)
    return true;
  if (Opcode >= X86::VADDPDrr && Opcode <= X86::VXORPDrr)
    return true;
  if (Opcode >= X86::VADDPDYrr && Opcode <= X86::VXORPDYrr)
    return true;
  if (Opcode >= X86::MMX_PADDBrr && Opcode <= X86::MMX_PXORrr)
    return true;

  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isReg()) {
      unsigned Reg = MO.getReg();
      if (X86::XMM0 <= Reg && Reg <= X86::XMM31)
        return true;
      if (X86::YMM0 <= Reg && Reg <= X86::YMM31)
        return true;
      if (X86::ZMM0 <= Reg && Reg <= X86::ZMM31)
        return true;
    }
  }

  return false;
}

} // namespace

static RegisterPass<SIMDCounterMachinePass>
    X("simd-counter-machine", "Count SIMD instructions", false, false);
