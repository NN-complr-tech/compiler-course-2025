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

using namespace llvm;

namespace {
    class FmaDecompose : public MachineFunctionPass {
    public:
        static char ID;
        FmaDecompose() : MachineFunctionPass(ID) {}

        bool runOnMachineFunction(MachineFunction& MF) override {
            const X86InstrInfo* TII =
                static_cast<const X86InstrInfo*>(MF.getSubtarget().getInstrInfo());
            MachineRegisterInfo& MRI = MF.getRegInfo();
            bool Changed = false;

            for (auto& MBB : MF) {
                SmallVector<MachineInstr*, 4> ToErase;

                for (auto& MI : MBB) {
                    unsigned Opcode = MI.getOpcode();
                    DebugLoc DL = MI.getDebugLoc();

                    unsigned MulOpc = 0, AddOpc = 0;

                    if (Opcode == X86::VFMADD132SSr || Opcode == X86::VFMADD213SSr ||
                        Opcode == X86::VFMADD231SSr) {
                        MulOpc = X86::VMULSSrr;
                        AddOpc = X86::VADDSSrr;
                    }
                    else if (Opcode == X86::VFMADD132PSr || Opcode == X86::VFMADD213PSr ||
                        Opcode == X86::VFMADD231PSr) {
                        MulOpc = X86::VMULPSrr;
                        AddOpc = X86::VADDPSrr;
                    }
                    else if (Opcode == X86::VFMADD132SDr || Opcode == X86::VFMADD213SDr ||
                        Opcode == X86::VFMADD231SDr) {
                        MulOpc = X86::VMULSDrr;
                        AddOpc = X86::VADDSDrr;
                    }
                    else if (Opcode == X86::VFMADD132PDr || Opcode == X86::VFMADD213PDr ||
                        Opcode == X86::VFMADD231PDr) {
                        MulOpc = X86::VMULPDrr;
                        AddOpc = X86::VADDPDrr;
                    }
                    else {
                        continue;
                    }

                    if (MI.getNumExplicitOperands() < 4)
                        continue;
                    if (!MI.getOperand(0).isReg() || !MI.getOperand(1).isReg() ||
                        !MI.getOperand(2).isReg() || !MI.getOperand(3).isReg())
                        continue;

                    Register Dest = MI.getOperand(0).getReg();
                    Register v1 = MI.getOperand(1).getReg();
                    Register v2 = MI.getOperand(2).getReg();
                    Register v3 = MI.getOperand(3).getReg();

                    const TargetRegisterClass* RC = MRI.getRegClass(v1);
                    Register MulTmp = MRI.createVirtualRegister(RC);

                    BuildMI(MBB, MI, DL, TII->get(MulOpc), MulTmp)
                        .addReg(v1)
                        .addReg(v2);
                    BuildMI(MBB, MI, DL, TII->get(AddOpc), Dest)
                        .addReg(v3)
                        .addReg(MulTmp);

                    ToErase.push_back(&MI);
                    Changed = true;
                }

                for (MachineInstr* MI : ToErase)
                    MI->eraseFromParent();
            }

            return Changed;
        }
    };
} // namespace

char FmaDecompose::ID = 0;
static RegisterPass<FmaDecompose>
X("decompose-fma", "Decompose FMA into MUL + ADD (SS/PS/SD/PD)", false, false);
