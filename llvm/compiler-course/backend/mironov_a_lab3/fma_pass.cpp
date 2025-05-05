#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {

class FMAPass : public MachineFunctionPass {
private:
  bool is_fma(MachineInstr &instraction) {
    unsigned type = instraction.getOpcode();
    return type == X86::VFMADD213SSr_Int || type == X86::VFMADD231SSr_Int ||
           type == X86::VFMADD132SSr_Int || type == X86::VFMADD213SSr ||
           type == X86::VFMADD231SSr || type == X86::VFMADD132SSr;
  }
  bool op_is_reg(MachineInstr &instraction) {
    return !instraction.getOperand(0).isReg() ||
           !instraction.getOperand(1).isReg() ||
           !instraction.getOperand(2).isReg() ||
           !instraction.getOperand(3).isReg();
  }

public:
  static char ID;
  FMAPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool flag = false;

    const X86InstrInfo *inst_info =
        MF.getSubtarget<X86Subtarget>().getInstrInfo();
    MachineRegisterInfo &reg_info = MF.getRegInfo();
    std::vector<MachineInstr *> operations;

    for (MachineBasicBlock &block : MF) {
      operations.clear();
      for (MachineInstr &instraction : block) {

        if (!is_fma(instraction) || op_is_reg(instraction) ||
            instraction.getNumExplicitOperands() <= 3) {
          continue;
        }

        Register r0 = instraction.getOperand(0).getReg();
        Register r1 = instraction.getOperand(1).getReg();
        Register r2 = instraction.getOperand(2).getReg();
        Register r3 = instraction.getOperand(3).getReg();

        const TargetRegisterClass *target =
            reg_info.getTargetRegisterInfo()->getRegClass(X86::FR32RegClassID);

        Register r4 = reg_info.createVirtualRegister(target);
        BuildMI(block, instraction, instraction.getDebugLoc(),
                inst_info->get(X86::MULSSrr), r4)
            .addReg(r1)
            .addReg(r2);
        BuildMI(block, instraction, instraction.getDebugLoc(),
                inst_info->get(X86::ADDSSrr), r0)
            .addReg(r4)
            .addReg(r3);

        operations.push_back(&instraction);
        flag = true;
      }

      for (auto *ops : operations)
        ops->eraseFromParent();
    }

    return flag;
  }
};

char FMAPass::ID = 0;

} // namespace

static llvm::RegisterPass<FMAPass>
    X("fma_pass", "Replacing FMA instructions by combination of ADD and MUL",
      false, false);