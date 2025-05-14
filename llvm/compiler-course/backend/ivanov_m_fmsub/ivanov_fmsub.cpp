#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class FMSubPass : public MachineFunctionPass {
public:
  static char ID;
  FMSubPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char FMSubPass::ID = 0;

bool FMSubPass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  if (!STI.hasFMA())
    return false;

  const TargetInstrInfo *TII = STI.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    auto E = MBB.end();
    for (auto MII = MBB.begin(); MII != E; ++MII) {
      MachineInstr &SubMI = *MII;
      unsigned SubOpc = SubMI.getOpcode();
      if (!(SubOpc == X86::SUBSDrr || SubOpc == X86::SUBSSrr ||
            SubOpc == X86::SUBPSrr || SubOpc == X86::SUBPDrr ||
            SubOpc == X86::VSUBPSYrr || SubOpc == X86::VSUBPDYrr ||
            SubOpc == X86::VSUBSDrr || SubOpc == X86::VSUBSSrr)) {
        continue;
      }
      // potential register - result of mul instruction
      Register TmpReg = SubMI.getOperand(2).getReg();
      // try to find mul instruction
      MachineInstr *TmpMI = MF.getRegInfo().getUniqueVRegDef(TmpReg);
      if (!TmpMI || !(TmpMI->getOpcode() == X86::MULSDrr ||
                      TmpMI->getOpcode() == X86::MULSSrr ||
                      TmpMI->getOpcode() == X86::MULPSrr ||
                      TmpMI->getOpcode() == X86::MULPDrr ||
                      TmpMI->getOpcode() == X86::VMULPSYrr ||
                      TmpMI->getOpcode() == X86::VMULPDYrr ||
                      TmpMI->getOpcode() == X86::VMULSDrr ||
                      TmpMI->getOpcode() == X86::VMULSSrr)) {
          continue;
        }

      MachineInstr &MulMI = *TmpMI;

      Register Dst = SubMI.getOperand(0).getReg();
      Register CReg = SubMI.getOperand(1).getReg();
      Register AReg = MulMI.getOperand(1).getReg();
      Register BReg = MulMI.getOperand(2).getReg();

      unsigned FmsubOpc;
      switch (SubOpc) {
      case X86::SUBSDrr:
        FmsubOpc = X86::VFMSUB213SDr;
        break;
      case X86::VSUBSDrr:
        FmsubOpc = X86::VFMSUB213SDr;
        break;
      case X86::SUBSSrr:
        FmsubOpc = X86::VFMSUB213SSr;
        break;
      case X86::VSUBSSrr:
        FmsubOpc = X86::VFMSUB213SSr;
        break;
      case X86::SUBPSrr:
        FmsubOpc = X86::VFMSUB213PSr;
        break;
      case X86::VSUBPSYrr:
        FmsubOpc = X86::VFMSUB213PSr;
        break;
      case X86::VSUBPDYrr:
        FmsubOpc = X86::VFMSUB213PDYr;
        break;
      case X86::SUBPDrr:
        FmsubOpc = X86::VFMSUB213PDYr;
        break;
      default:
        continue;
      }

      BuildMI(MBB, SubMI, SubMI.getDebugLoc(), TII->get(FmsubOpc))
          .addReg(Dst, RegState::Define)
          .addReg(CReg)
          .addReg(AReg)
          .addReg(BReg)
          .addImm(0)
          .addReg(X86::MXCSR, RegState::Implicit)
          .addReg(X86::MXCSR, RegState::Implicit);

      auto NextSubIt = std::next(MII);
      SubMI.eraseFromParent();

      for (auto I = MBB.begin(); I != MBB.end(); ++I) {
        if (&*I == &MulMI) {
          I->eraseFromParent();
          break;
        }
      }

      MII = NextSubIt;
      E = MBB.end();
      Changed = true;
    }
  }

  return Changed;
}
} // namespace

static RegisterPass<FMSubPass> X("fmsub_ivanov", "fused multiply–subtract pass",
                                 false, false);
