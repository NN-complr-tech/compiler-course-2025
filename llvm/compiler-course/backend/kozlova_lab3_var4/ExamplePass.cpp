#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "mul_sub"

using namespace llvm;

namespace {
class MulSubPass : public MachineFunctionPass {
public:
  static char ID;
  MulSubPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (MachineBasicBlock &MBB : MF) {
      for (MachineBasicBlock::iterator MI = MBB.begin(), ME = MBB.end();
           MI != ME;) {
        MachineInstr &Sub = *MI++;

        if (!isFloatingPointSubtraction(Sub))
          continue;

        MachineInstr *Mul = findMultiplyInstruction(MRI, Sub);
        if (!Mul)
          continue;

        fuseMultiplySubtract(MBB, MI, Sub, Mul, TII);
        Changed = true;
      }
    }

    return Changed;
  }

private:
  bool isFloatingPointSubtraction(const MachineInstr &MI) const {
    unsigned Opcode = MI.getOpcode();
    return Opcode == X86::VSUBSSrr || Opcode == X86::VSUBSDrr;
  }

  MachineInstr *findMultiplyInstruction(MachineRegisterInfo &MRI,
                                        const MachineInstr &Sub) const {
    Register SubOp2 = Sub.getOperand(2).getReg();
    MachineInstr *Mul = MRI.getUniqueVRegDef(SubOp2);
    if (!Mul)
      return nullptr;

    unsigned MulOpcode = Mul->getOpcode();
    return (MulOpcode == X86::VMULSSrr || MulOpcode == X86::VMULSDrr) ? Mul
                                                                      : nullptr;
  }

  void fuseMultiplySubtract(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator &MI, MachineInstr &Sub,
                            MachineInstr *Mul, const X86InstrInfo *TII) {
    Register SubDst = Sub.getOperand(0).getReg();
    Register SubOp1 = Sub.getOperand(1).getReg();
    Register MulOp1 = Mul->getOperand(1).getReg();
    Register MulOp2 = Mul->getOperand(2).getReg();
    DebugLoc DL = Sub.getDebugLoc();

    unsigned NewOpcode = (Sub.getOpcode() == X86::VSUBSSrr)
                             ? X86::VFNMADD213SSr
                             : X86::VFNMADD213SDr;

    BuildMI(MBB, &Sub, DL, TII->get(NewOpcode), SubDst)
        .addReg(SubOp1)
        .addReg(MulOp1)
        .addReg(MulOp2)
        .addImm(0)
        .addReg(X86::MXCSR, RegState::Implicit);

    Sub.eraseFromParent();
    Mul->eraseFromParent();

    MI = MBB.begin();
  }
};
char MulSubPass::ID = 0;
} // namespace

static RegisterPass<MulSubPass>
    X("mul_sub", "Fuse MUL + SUB into VFMSUB213 instructions", false, false);
