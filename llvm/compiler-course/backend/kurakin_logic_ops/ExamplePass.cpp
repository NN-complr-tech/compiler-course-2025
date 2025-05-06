#include "MCTargetDesc/X86MCTargetDesc.h"
#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Instruction.h"

using namespace llvm;

namespace {
class LogicOpsPass : public MachineFunctionPass {
public:
  static char ID;
  LogicOpsPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char LogicOpsPass::ID = 0;

bool LogicOpsPass::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  const llvm::X86InstrInfo *TII =
      MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();
  const llvm::MachineRegisterInfo *MRI = &MF.getRegInfo();

  for (auto &MBB : MF) {
    for (auto &MI : llvm::make_early_inc_range(MBB)) {
      if (MI.getOpcode() == llvm::X86::PANDrr) {
        MachineOperand &MIOp0 = MI.getOperand(0);
        MachineOperand &MIOp1 = MI.getOperand(1);
        MachineOperand &MIOp2 = MI.getOperand(2);
        if (MIOp1.isReg() && MIOp2.isReg()) {
          llvm::MachineInstr *DefMIOp1 = MRI->getUniqueVRegDef(MIOp1.getReg());
          llvm::MachineInstr *DefMIOp2 = MRI->getUniqueVRegDef(MIOp2.getReg());
          if (MIOp1.getReg() == MIOp2.getReg()) {
            llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                          TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                .addReg(MIOp1.getReg());
            MI.eraseFromParent();
            Changed = true;
          } else if (DefMIOp1) {
            if (DefMIOp1->getOpcode() == llvm::X86::V_SET0) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp1.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
            if (DefMIOp1->getOpcode() == llvm::X86::V_SETALLONES) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp2.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
          } else if (DefMIOp2) {
            if (DefMIOp2->getOpcode() == llvm::X86::V_SET0) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp2.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
            if (DefMIOp2->getOpcode() == llvm::X86::V_SETALLONES) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp1.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
          }
        }
      }
    }
  }

  for (auto &MBB : MF) {
    for (auto &MI : llvm::make_early_inc_range(MBB)) {
      if (MI.getOpcode() == llvm::X86::PORrr) {
        MachineOperand &MIOp0 = MI.getOperand(0);
        MachineOperand &MIOp1 = MI.getOperand(1);
        MachineOperand &MIOp2 = MI.getOperand(2);
        if (MIOp1.isReg() && MIOp2.isReg()) {
          llvm::MachineInstr *DefMIOp1 = MRI->getUniqueVRegDef(MIOp1.getReg());
          llvm::MachineInstr *DefMIOp2 = MRI->getUniqueVRegDef(MIOp2.getReg());
          if (MIOp1.getReg() == MIOp2.getReg()) {
            llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                          TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                .addReg(MIOp1.getReg());
            MI.eraseFromParent();
            Changed = true;
          } else if (DefMIOp1) {
            if (DefMIOp1->getOpcode() == llvm::X86::V_SET0) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp2.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
            if (DefMIOp1->getOpcode() == llvm::X86::V_SETALLONES) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp1.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
          } else if (DefMIOp2) {
            if (DefMIOp2->getOpcode() == llvm::X86::V_SET0) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp1.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
            if (DefMIOp2->getOpcode() == llvm::X86::V_SETALLONES) {
              llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                            TII->get(llvm::TargetOpcode::COPY), MIOp0.getReg())
                  .addReg(MIOp2.getReg());
              MI.eraseFromParent();
              Changed = true;
            }
          }
        }
      }
    }
  }
  for (auto &MBB : MF) {
    for (auto &MI : llvm::make_early_inc_range(MBB)) {
      unsigned NewOpc = 0;
      if (MI.getOpcode() == llvm::X86::PANDrr)
        NewOpc = llvm::X86::VPANDrr;
      if (MI.getOpcode() == llvm::X86::PORrr)
        NewOpc = llvm::X86::VPORrr;
      if (MI.getOpcode() == llvm::X86::PXORrr)
        NewOpc = llvm::X86::VPXORrr;
      if (MI.getOpcode() == llvm::X86::PANDNrr)
        NewOpc = llvm::X86::VPANDNrr;
      if (NewOpc) {
        MachineOperand &MIOp0 = MI.getOperand(0);
        MachineOperand &MIOp1 = MI.getOperand(1);
        MachineOperand &MIOp2 = MI.getOperand(2);
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

static RegisterPass<LogicOpsPass> X("kurakin-logic-ops-x86", "logic_ops_pass",
                                    false, false);
