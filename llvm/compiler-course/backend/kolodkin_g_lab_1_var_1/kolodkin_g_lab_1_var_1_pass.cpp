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
            if (inst2->getOpcode() == llvm::X86::PANDrr) {
              if (inst1->getOperand(0).getReg() ==
                  inst2->getOperand(1).getReg()) {
                BuildMI(MBB, inst2, inst2->getDebugLoc(),
                        TII->get(llvm::X86::PANDrr),
                        inst1->getOperand(0).getReg())
                    .addReg(inst2->getOperand(2).getReg())
                    .addReg(inst1->getOperand(2).getReg());

                BuildMI(MBB, inst2, inst2->getDebugLoc(),
                        TII->get(llvm::X86::PANDrr),
                        inst2->getOperand(0).getReg())
                    .addReg(inst1->getOperand(0).getReg())
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

        switch (inst->getOpcode()) {
        case llvm::X86::PORrr:
          replaceInstruction(MF, MBB, inst, llvm::X86::VPORrr);
          modified = true;
          break;

        case llvm::X86::PXORrr:
          replaceInstruction(MF, MBB, inst, llvm::X86::VPXORrr);
          modified = true;
          break;

        case llvm::X86::PANDrr:
          replaceInstruction(MF, MBB, inst, llvm::X86::VPANDrr);
          modified = true;
          break;

        default:
          break;
        }
      }
    }
    return modified;
  }
  void replaceInstruction(llvm::MachineFunction &MF, llvm::MachineBasicBlock &MBB, 
                          llvm::MachineInstr *inst, unsigned newOpcode) {
      auto &TII = *MF.getSubtarget<llvm::X86Subtarget>().getInstrInfo();
      BuildMI(MBB, inst, inst->getDebugLoc(), 
              TII.get(newOpcode), inst->getOperand(0).getReg())
          .addReg(inst->getOperand(1).getReg())
          .addReg(inst->getOperand(2).getReg());

      inst->eraseFromParent();
  }
};

char LogicalChainOptimization::ID = 0;
} // namespace

static llvm::RegisterPass<LogicalChainOptimization>
    X("logical-opt-x86", "Logical Optimization Pass", false, false);
