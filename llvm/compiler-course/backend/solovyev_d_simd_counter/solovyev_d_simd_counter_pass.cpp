#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

namespace {

bool isSIMDInstruction(const MachineInstr &MI, const MachineFunction &MF) {
  if (MI.getOpcode() == TargetOpcode::COPY) {
    return false;
  }
  for (const MachineOperand &MO : MI.operands()) {
    if (!MO.isReg())
      continue;

    Register R = MO.getReg();

    if (!R.isValid() || !R.isVirtual())
      continue;

    const TargetRegisterClass *RC = MF.getRegInfo().getRegClass(R);

    switch (RC->getID()) {
    case X86::VR128RegClassID:
    case X86::VR256RegClassID:
    case X86::VR512RegClassID:
      return true;
    default:
      break;
    }
  }
  return false;
}

class SimdCounterPass : public MachineFunctionPass {
public:
  static char ID;
  SimdCounterPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char SimdCounterPass::ID = 0;

bool SimdCounterPass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = STI.getInstrInfo();

  for (auto &MBB : MF) {
    for (auto MI = MBB.begin(); MI != MBB.end(); ++MI) {
      MachineInstr &Inst = *MI;
      // llvm::outs() << "Checking " << TII->getName(MI->getOpcode()) << '\n';
      if (!isSIMDInstruction(Inst, MF))
        continue;
      // llvm::outs() << TII->getName(MI->getOpcode()) << " is vector!" << '\n';
      DebugLoc DL = MI->getDebugLoc();
      MachineBasicBlock::iterator InsertPos = std::next(MI);

      BuildMI(MBB, InsertPos, DL, TII->get(X86::MOV64rm), X86::RAX)
          .addReg(0)
          .addImm(1)
          .addReg(0)
          .addExternalSymbol("simd_counter")
          .addReg(0);
      BuildMI(MBB, InsertPos, DL, TII->get(X86::ADD64ri32), X86::RAX)
          .addReg(X86::RAX)
          .addImm(1);

      BuildMI(MBB, InsertPos, DL, TII->get(X86::MOV64mr))
          .addReg(0)
          .addImm(1)
          .addReg(0)
          .addExternalSymbol("simd_counter")
          .addReg(0)
          .addReg(X86::RAX);
    }
  }

  return true;
}
} // namespace

static RegisterPass<SimdCounterPass>
    X("simd-counter", "SIMD Instruction Counter Pass", false, false);