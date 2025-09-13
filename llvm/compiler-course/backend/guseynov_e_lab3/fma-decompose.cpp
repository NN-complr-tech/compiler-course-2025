#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {
class FMADecomposePass : public MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();

    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto MI = MBB.begin(); MI != MBB.end(); ) {
        MachineInstr &Instr = *MI++;
        unsigned Opc = Instr.getOpcode();

        if (Opc == X86::VFMADD132PSrr || Opc == X86::VFMADD132PDrr) {
          LLVM_DEBUG(dbgs() << "Found FMA in " << MF.getName() << "\n");

          Register Src0 = Instr.getOperand(1).getReg();
          Register Src1 = Instr.getOperand(2).getReg();
          Register Src2 = Instr.getOperand(3).getReg();

          Register TmpMul = MF.getRegInfo().createVirtualRegister(
              &X86::VR128RegClass);
          Register TmpAdd = MF.getRegInfo().createVirtualRegister(
              &X86::VR128RegClass);

          BuildMI(MBB, Instr, Instr.getDebugLoc(), TII->get(X86::VMULPSrr), TmpMul)
              .addReg(Src0)
              .addReg(Src1);

          BuildMI(MBB, Instr, Instr.getDebugLoc(), TII->get(X86::VADDPSrr), TmpAdd)
              .addReg(Src2)
              .addReg(TmpMul);

          Instr.getOperand(0).setReg(TmpAdd);

          Instr.eraseFromParent();

          Changed = true;
        }
      }
    }

    return Changed;
  }
};
} // namespace

char FMADecomposePass::ID = 0;

static RegisterPass<FMADecomposePass>
    X("fma-decompose", "Decompose FMA into MUL+ADD", false, false);
