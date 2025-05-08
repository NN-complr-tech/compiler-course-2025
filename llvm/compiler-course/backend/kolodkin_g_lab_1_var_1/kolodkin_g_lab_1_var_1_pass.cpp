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
            for (auto I = MBB.begin(), E = MBB.end(); I != E; ) {
                llvm::MachineInstr *inst = &*I++;

                if (inst->getOpcode() == llvm::X86::PORrr) {
                    if (std::distance(I, E) < 2) {
                        continue;
                    }

                    llvm::MachineInstr *nextInst1 = &*I;
                    llvm::MachineInstr *nextInst2 = &*(std::next(I));


                    if (nextInst1->getOpcode() == llvm::X86::PORrr &&
                        nextInst2->getOpcode() == llvm::X86::PANDrr) {

                        llvm::Register reg1_0 = inst->getOperand(0).getReg();
                        llvm::Register reg1_1 = inst->getOperand(1).getReg();
                        llvm::Register reg1_2 = inst->getOperand(2).getReg();
                        llvm::Register reg2_0 = nextInst1->getOperand(0).getReg();
                        llvm::Register reg2_1 = nextInst1->getOperand(1).getReg();
                        llvm::Register reg2_2 = nextInst1->getOperand(2).getReg();
                        llvm::Register reg3_0 = nextInst2->getOperand(0).getReg();
                        llvm::Register reg3_1 = nextInst2->getOperand(1).getReg();
                        llvm::Register reg3_2 = nextInst2->getOperand(2).getReg();

                        if ((reg1_1 == reg2_1 || reg1_2 == reg2_2) && (reg3_1 == reg1_0 && reg3_2 == reg2_0)) {
                            llvm::Register resReg1, resReg2, resReg3;
                            if (reg1_1 == reg2_1){
                                resReg1 = reg1_2;
                                resReg2 = reg2_2;
                                resReg3 = reg1_1;
                            } else if (reg1_2 == reg2_2) {
                                resReg1 = reg1_1;
                                resReg2 = reg2_1;
                                resReg3 = reg1_2;
                            }
                            BuildMI(MBB, inst, inst->getDebugLoc(),
                                    TII->get(llvm::X86::PANDrr), reg2_0)
                                .addReg(resReg1)
                                .addReg(resReg2);

                            BuildMI(MBB, inst, inst->getDebugLoc(),
                                    TII->get(llvm::X86::PORrr), reg3_0)
                                .addReg(resReg3)
                                .addReg(reg2_0);


                            toErase.push_back(inst);
                            toErase.push_back(nextInst1);
                            toErase.push_back(nextInst2);

                            modified = true;
                        }
                    }
                }
            }
            for (auto *instr : toErase) {
                instr->eraseFromParent();
            }
        }

        for (auto &MBB : MF) {
            for (auto I = MBB.begin(), E = MBB.end(); I != E; ) {
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
void replaceInstruction(llvm::MachineFunction &MF, llvm::MachineBasicBlock &MBB, llvm::MachineInstr *inst, unsigned newOpcode) {
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

static llvm::RegisterPass<LogicalChainOptimization> X("logical-opt-x86",
                                               "Logical Optimization Pass",
                                               false, false);
