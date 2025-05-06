#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class LogicalCombinerPass : public MachineFunctionPass {
public:
  static char ID;
  LogicalCombinerPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char LogicalCombinerPass::ID = 0;

bool LogicalCombinerPass::runOnMachineFunction(MachineFunction &MF) {
  llvm::outs() << MF.getName() << '\n';

  const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = STI.getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  bool Changed = false;

  for (auto &MBB : MF) {
    for (auto MI = MBB.begin(); MI != MBB.end();) {
      MachineInstr &AndInstr = *MI;
      unsigned AndOpc = AndInstr.getOpcode();

      if (AndOpc != X86::ANDPSrr && AndOpc != X86::VPANDrr) {
        ++MI;
        continue;
      }

      if (!AndInstr.getOperand(0).isReg() || !AndInstr.getOperand(1).isReg() ||
          !AndInstr.getOperand(2).isReg()) {
        ++MI;
        continue;
      }

      Register AndDst = AndInstr.getOperand(0).getReg();
      Register AndOp1 = AndInstr.getOperand(1).getReg();
      Register AndOp2 = AndInstr.getOperand(2).getReg();

      auto NextMI = std::next(MI);
      if (NextMI == MBB.end()) {
        ++MI;
        continue;
      }

      MachineInstr &OrInstr = *NextMI;
      unsigned OrOpc = OrInstr.getOpcode();

      if ((OrOpc != X86::ORPSrr && OrOpc != X86::VPORrr) ||
          !OrInstr.getOperand(1).isReg() ||
          OrInstr.getOperand(1).getReg() != AndDst ||
          !OrInstr.getOperand(0).isReg() || !OrInstr.getOperand(2).isReg()) {
        ++MI;
        continue;
      }

      Register OrDst = OrInstr.getOperand(0).getReg();
      Register OrOther = OrInstr.getOperand(2).getReg();

      DebugLoc DL = AndInstr.getDebugLoc();

      const TargetRegisterClass *RC = MRI.getRegClass(AndOp1);
      if (!RC)
        RC = MRI.getRegClass(AndOp2);
      if (!RC)
        continue;

      Register TmpReg = MRI.createVirtualRegister(RC);

      BuildMI(MBB, AndInstr, DL, TII->get(X86::VPANDrr), TmpReg)
          .addReg(AndOp1)
          .addReg(AndOp2);

      BuildMI(MBB, OrInstr, DL, TII->get(X86::VPORrr), OrDst)
          .addReg(TmpReg)
          .addReg(OrOther);

      MI = MBB.erase(MI);
      MBB.erase(NextMI);

      Changed = true;
    }
  }

  return Changed;
}
} // namespace

static RegisterPass<LogicalCombinerPass>
    X("logic-combiner-x86", "Combine logical ops into AVX", false, false);
