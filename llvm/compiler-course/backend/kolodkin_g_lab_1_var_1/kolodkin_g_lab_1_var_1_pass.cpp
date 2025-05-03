#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace {
class LogicalChainOptimization : public llvm::MachineFunctionPass {
public:
  static char ID;
  LogicalChainOptimization() : MachineFunctionPass(ID) {}
  const llvm::TargetInstrInfo *TII;

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    bool modified = false;
    TII = MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();

    for (auto &MBB : MF) {
      std::vector<llvm::MachineInstr *> toErase;
      for (auto I = MBB.begin(), E = MBB.end(); I != E;) {
        llvm::MachineInstr *inst1 = &*I++;

        if (inst1->getOpcode() == llvm::X86::PANDrr) {
          if (I != E) {
            llvm::MachineInstr *inst2 = &*I;
            if (inst2->getOpcode() == llvm::X86::PORrr) {
              if (inst1->getOperand(0).getReg() ==
                  inst2->getOperand(1).getReg()) {
                llvm::Register destRegPAND = inst1->getOperand(0).getReg();
                llvm::Register srcRegPAND1 = inst1->getOperand(1).getReg();
                llvm::Register srcRegPAND2 = inst2->getOperand(1).getReg();
                llvm::Register destRegPOR = inst2->getOperand(0).getReg();

                BuildMI(MBB, inst2, inst2->getDebugLoc(),
                        TII->get(llvm::X86::PORrr), destRegPAND)
                   .addReg(inst2->getOperand(2).getReg())
                   .addReg(inst1->getOperand(2).getReg());

                BuildMI(MBB, inst2, inst2->getDebugLoc(),
                        TII->get(llvm::X86::PANDrr), destRegPOR)
                   .addReg(destRegPAND)
                   .addReg(inst1->getOperand(1).getReg());

                toErase.push_back(inst1);
                toErase.push_back(inst2);

                modified = true;
              }
            }
          }
        }
      }
      for (auto *instr : toErase) {
        instr->eraseFromParent();
      }
    }

    for (auto &MBB : MF) {
      for (auto I = MBB.begin(), E = MBB.end(); I != E;) {
        llvm::MachineInstr *inst = &*I++;

        if (inst->getOpcode() == llvm::X86::PORrr) {
          auto &TII = *MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();
          BuildMI(MBB, inst, inst->getDebugLoc(), TII.get(llvm::X86::VPORrr),
                  inst->getOperand(0).getReg())
             .addReg(inst->getOperand(1).getReg())
             .addReg(inst->getOperand(2).getReg());

          inst->eraseFromParent();
          modified = true;
        }

        if (inst->getOpcode() == llvm::X86::PXORrr) {
          auto &TII = *MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();
          BuildMI(MBB, inst, inst->getDebugLoc(), TII.get(llvm::X86::VPXORrr),
                  inst->getOperand(0).getReg())
             .addReg(inst->getOperand(1).getReg())
             .addReg(inst->getOperand(2).getReg());

          inst->eraseFromParent();
          modified = true;
        }

        if (inst->getOpcode() == llvm::X86::PANDrr) {
          auto &TII = *MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();
          BuildMI(MBB, inst, inst->getDebugLoc(), TII.get(llvm::X86::VPANDrr),
                  inst->getOperand(0).getReg())
             .addReg(inst->getOperand(1).getReg())
             .addReg(inst->getOperand(2).getReg());

          inst->eraseFromParent();
          modified = true;
        }
      }
    }
    return modified;
  }
};

char LogicalChainOptimization::ID = 0;
} // namespace

static llvm::RegisterPass<LogicalChainOptimization>
    X("logical-opt-x86", "Logical Optimization Pass", false, false);
