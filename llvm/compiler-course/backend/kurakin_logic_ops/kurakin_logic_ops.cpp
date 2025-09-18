#include "MCTargetDesc/X86MCTargetDesc.h"
#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Instruction.h"

namespace {
class LogicOpsPass : public llvm::MachineFunctionPass {
  const llvm::X86InstrInfo *TII;
  const llvm::MachineRegisterInfo *MRI;

  bool optimizeLogicOpWithConstant(llvm::MachineBasicBlock &MBB,
                                   llvm::MachineInstr &MI,
                                   llvm::MachineOperand &MIOp0,
                                   llvm::MachineOperand &MIOp1,
                                   llvm::MachineOperand &MIOp2) {
    llvm::MachineInstr *DefMIOp;
    if (MI.getOpcode() == llvm::X86::PANDrr)
      DefMIOp = MRI->getUniqueVRegDef(MIOp1.getReg());
    else
      DefMIOp = MRI->getUniqueVRegDef(MIOp2.getReg());

    bool Changed = false;
    if (!DefMIOp)
      return Changed;

    if (DefMIOp->getOpcode() == llvm::X86::V_SET0) {
      llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                    TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
          .addReg(MIOp1.getReg());
      MI.eraseFromParent();
      Changed = true;
    } else if (DefMIOp->getOpcode() == llvm::X86::V_SETALLONES) {
      llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                    TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
          .addReg(MIOp2.getReg());
      MI.eraseFromParent();
      Changed = true;
    }
    return Changed;
  }

public:
  static char ID;
  LogicOpsPass() : llvm::MachineFunctionPass(ID) {}
  bool runOnMachineFunction(llvm::MachineFunction &MF) override;
};

char LogicOpsPass::ID = 0;

bool LogicOpsPass::runOnMachineFunction(llvm::MachineFunction &MF) {
  bool Changed = false;
  TII = MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();
  MRI = &MF.getRegInfo();

  for (auto &MBB : MF) {
    for (auto &MI : llvm::make_early_inc_range(MBB)) {
      if (MI.getOpcode() != llvm::X86::PANDrr &&
          MI.getOpcode() != llvm::X86::PORrr)
        continue;
      llvm::MachineOperand &MIOp0 = MI.getOperand(0);
      llvm::MachineOperand &MIOp1 = MI.getOperand(1);
      llvm::MachineOperand &MIOp2 = MI.getOperand(2);
      if (!MIOp1.isReg() || !MIOp2.isReg())
        continue;
      if (MIOp1.getReg() == MIOp2.getReg()) {
        llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                      TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
            .addReg(MIOp1.getReg());
        MI.eraseFromParent();
        Changed = true;
      } else {
        Changed = optimizeLogicOpWithConstant(MBB, MI, MIOp0, MIOp1, MIOp2) ||
                  optimizeLogicOpWithConstant(MBB, MI, MIOp0, MIOp2, MIOp1);
      }
    }
  }

  for (auto &MBB : MF) {
    for (auto &MI : llvm::make_early_inc_range(MBB)) {
      unsigned NewOpc = 0;
      switch (MI.getOpcode()) {
      case llvm::X86::PANDrr:
        NewOpc = llvm::X86::VPANDrr;
        break;
      case llvm::X86::PORrr:
        NewOpc = llvm::X86::VPORrr;
        break;
      case llvm::X86::PXORrr:
        NewOpc = llvm::X86::VPXORrr;
        break;
      case llvm::X86::PANDNrr:
        NewOpc = llvm::X86::VPANDNrr;
        break;
      }
      if (NewOpc) {
        llvm::MachineOperand &MIOp0 = MI.getOperand(0);
        llvm::MachineOperand &MIOp1 = MI.getOperand(1);
        llvm::MachineOperand &MIOp2 = MI.getOperand(2);
        llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(NewOpc),
                      MIOp0.getReg())
            .addReg(MIOp1.getReg())
            .addReg(MIOp2.getReg());
        MI.eraseFromParent();
        Changed = true;
      }
    }
  }
  return Changed;
}
} // namespace

static llvm::RegisterPass<LogicOpsPass> X("kurakin-logic-ops-x86",
                                          "logic_ops_pass", false, false);
