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

#define DEBUG_TYPE "mul_sub_solovev"

using namespace llvm;

namespace {

class FMSUBComposePass : public MachineFunctionPass {
public:
  static char ID;
  FMSUBComposePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    if (!ST.hasFMA())
      return false;
    const X86InstrInfo *TII = ST.getInstrInfo();
    auto &MRI = MF.getRegInfo();
    bool Changed = false;
    for (auto &MBB : MF) {
      for (auto MI = MBB.instr_begin(), ME = MBB.instr_end(); MI != ME;) {
        MachineInstr &Sub = *MI++;
        if (Sub.isMetaInstruction())
          continue;
        unsigned SubOpcode = Sub.getOpcode();
        if (SubOpcode != X86::VSUBSSrr && SubOpcode != X86::VSUBSDrr)
          continue;
        Register SubDst = Sub.getOperand(0).getReg();
        Register SubOp1 = Sub.getOperand(1).getReg();
        Register SubOp2 = Sub.getOperand(2).getReg();
        if (!MRI.hasOneUse(SubOp2))
          continue;
        MachineInstr *Mul = MRI.getUniqueVRegDef(SubOp2);
        if (!Mul)
          continue;
        unsigned MulOpcode = Mul->getOpcode();
        if (MulOpcode != X86::VMULSSrr && MulOpcode != X86::VMULSDrr)
          continue;
        Register MulOp1 = Mul->getOperand(1).getReg();
        Register MulOp2 = Mul->getOperand(2).getReg();
        DebugLoc DL = Sub.getDebugLoc();
        MachineBasicBlock &MBBRef = *Sub.getParent();
        unsigned NewOpcode = (SubOpcode == X86::VSUBSSrr) ? X86::VFNMADD213SSr
                                                          : X86::VFNMADD213SDr;
        BuildMI(MBBRef, &Sub, DL, TII->get(NewOpcode), SubDst)
            .addReg(SubOp1)
            .addReg(MulOp1)
            .addReg(MulOp2)
            .addImm(0)
            .addReg(X86::MXCSR, RegState::Implicit);
        Sub.eraseFromParent();
        Mul->eraseFromParent();
        Changed = true;
      }
    }
    return Changed;
  }
};
char FMSUBComposePass::ID = 0;

} // namespace

static RegisterPass<FMSUBComposePass>
    X("mul_sub_solovev", "Fuse MUL + SUB into VFNMADD213 instructions", false,
      false);
