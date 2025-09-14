#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"

#define DEBUG_TYPE "fma-decompose-x86"

namespace {

class FmaDecompositionPass : public llvm::MachineFunctionPass {
public:
  static char ID;
  FmaDecompositionPass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &ST = MF.getSubtarget<llvm::X86Subtarget>();

    const llvm::X86InstrInfo *TII = ST.getInstrInfo();
    llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;
      for (auto &MI : MBB) {
        unsigned Opc = MI.getOpcode();
        if (Opc == llvm::X86::VFMADD132SSr || Opc == llvm::X86::VFMADD213SSr ||
            Opc == llvm::X86::VFMADD231SSr ||
            Opc == llvm::X86::VFMADD132SSr_Int ||
            Opc == llvm::X86::VFMADD213SSr_Int ||
            Opc == llvm::X86::VFMADD231SSr_Int) {

          if (MI.getNumExplicitOperands() < 4)
            continue;

          if (!MI.getOperand(0).isReg() || !MI.getOperand(1).isReg() ||
              !MI.getOperand(2).isReg() || !MI.getOperand(3).isReg())
            continue;

          llvm::Register Dest = MI.getOperand(0).getReg();
          llvm::Register Src1 = MI.getOperand(1).getReg();
          llvm::Register Src2 = MI.getOperand(2).getReg();
          llvm::Register Src3 = MI.getOperand(3).getReg();

          const llvm::TargetRegisterClass *RC =
              MRI.getTargetRegisterInfo()->getRegClass(
                  llvm::X86::FR32RegClassID);

          llvm::Register Tmp = MRI.createVirtualRegister(RC);

          llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(llvm::X86::MULSSrr),
                        Tmp)
              .addReg(Src1)
              .addReg(Src2);

          llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(llvm::X86::ADDSSrr),
                        Dest)
              .addReg(Tmp)
              .addReg(Src3);

          ToErase.push_back(&MI);
          Changed = true;
        }
      }
      for (auto *Old : ToErase)
        Old->eraseFromParent();
    }

    return Changed;
  }
};

char FmaDecompositionPass::ID = 0;

} // namespace

static llvm::RegisterPass<FmaDecompositionPass>
    X("fma-decompose-x86", "Decompose FMA to MUL + ADD", false, false);
