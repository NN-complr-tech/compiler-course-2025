#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace {

class SIMDCounterMachinePass : public MachineFunctionPass {
public:
  static char ID;
  SIMDCounterMachinePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool doInitialization(Module &M) override;
  StringRef getPassName() const override { return "SIMD Instruction Counter"; }

private:
  bool isX86SIMDInstruction(const MachineInstr &MI);
  void insertCounterUpdate(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator &MI);

  GlobalVariable *simdCounterVar = nullptr;
};

char SIMDCounterMachinePass::ID = 0;

bool SIMDCounterMachinePass::doInitialization(Module &M) {
  simdCounterVar = M.getGlobalVariable("simd_counter");
  if (!simdCounterVar) {
    simdCounterVar = new GlobalVariable(
        M, Type::getInt64Ty(M.getContext()), false,
        GlobalValue::ExternalLinkage,
        ConstantInt::get(Type::getInt64Ty(M.getContext()), 0), "simd_counter");
  }
  return true;
}

bool SIMDCounterMachinePass::runOnMachineFunction(MachineFunction &MF) {
  if (MF.empty() || !simdCounterVar)
    return false;

  bool modified = false;

  for (auto &MBB : MF) {
    auto MI = MBB.begin();
    while (MI != MBB.end()) {
      auto nextMI = std::next(MI);

      if (isX86SIMDInstruction(*MI)) {
        insertCounterUpdate(MBB, MI);
        modified = true;
      }

      MI = nextMI;
    }
  }

  return modified;
}

void SIMDCounterMachinePass::insertCounterUpdate(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator &MI) {
  const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI->getDebugLoc();

  auto insertAfter = std::next(MI);

  BuildMI(MBB, insertAfter, DL, TII->get(X86::MOV64rm), X86::RAX)
      .addGlobalAddress(simdCounterVar)
      .addReg(0)
      .addImm(1)
      .addReg(0)
      .addImm(0)
      .addReg(0);

  BuildMI(MBB, insertAfter, DL, TII->get(X86::ADD64ri32), X86::RAX)
      .addReg(X86::RAX)
      .addImm(1);

  BuildMI(MBB, insertAfter, DL, TII->get(X86::MOV64mr))
      .addGlobalAddress(simdCounterVar)
      .addReg(0)
      .addImm(1)
      .addReg(0)
      .addImm(0)
      .addReg(0)
      .addReg(X86::RAX);
}

bool SIMDCounterMachinePass::isX86SIMDInstruction(const MachineInstr &MI) {
  if (MI.isMetaInstruction() || MI.isLabel() || MI.isDebugInstr()) {
    return false;
  }

  unsigned Opcode = MI.getOpcode();

  if (Opcode >= X86::ADDPSrr && Opcode <= X86::XORPSrr)
    return true;
  if (Opcode >= X86::ADDPDrr && Opcode <= X86::XORPDrr)
    return true;

  if (Opcode >= X86::VADDPSrr && Opcode <= X86::VXORPSrr)
    return true;
  if (Opcode >= X86::VADDPDrr && Opcode <= X86::VXORPDrr)
    return true;

  if (Opcode >= X86::VADDPSYrr && Opcode <= X86::VXORPSYrr)
    return true;
  if (Opcode >= X86::VADDPDYrr && Opcode <= X86::VXORPDYrr)
    return true;

  if (Opcode >= X86::MMX_PADDBrr && Opcode <= X86::MMX_PXORrr)
    return true;

  if (Opcode >= X86::VADDPSZrr && Opcode <= X86::VXORPSZrr)
    return true;
  if (Opcode >= X86::VADDPDZrr && Opcode <= X86::VXORPDZrr)
    return true;

  if (Opcode == X86::MOVAPSrm || Opcode == X86::MOVAPSmr ||
      Opcode == X86::MOVAPDrm || Opcode == X86::MOVAPDmr ||
      Opcode == X86::VMOVAPSrm || Opcode == X86::VMOVAPSmr ||
      Opcode == X86::VMOVAPDrm || Opcode == X86::VMOVAPDmr ||
      Opcode == X86::VMOVAPSZrm || Opcode == X86::VMOVAPSZmr ||
      Opcode == X86::VMOVAPDZrm || Opcode == X86::VMOVAPDZmr) {
    return true;
  }

  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isReg()) {
      unsigned Reg = MO.getReg();
      if (X86::XMM0 <= Reg && Reg <= X86::XMM31)
        return true;
      if (X86::YMM0 <= Reg && Reg <= X86::YMM31)
        return true;
      if (X86::ZMM0 <= Reg && Reg <= X86::ZMM31)
        return true;
      if (X86::MM0 <= Reg && Reg <= X86::MM7)
        return true;
    }
  }

  return false;
}

} // namespace

static RegisterPass<SIMDCounterMachinePass>
    X("simd-counter-machine", "Count SIMD instructions", false, false);
