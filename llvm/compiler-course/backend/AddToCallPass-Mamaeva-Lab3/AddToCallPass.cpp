#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "add-to-call"

using namespace llvm;

namespace {

class AddToCallPass : public MachineFunctionPass {
public:
  static char ID;

  AddToCallPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    const TargetRegisterInfo *TRI = ST.getRegisterInfo();
    bool Changed = false;

    Function &F = MF.getFunction();
    Module *M = F.getParent();

    Function *AddFunc = M->getFunction("my_add");
    if (!AddFunc || AddFunc->isDeclaration()) {
      LLVM_DEBUG(dbgs() << "Function my_add not found or is declaration\n");
      return false;
    }

    for (auto &MBB : MF) {
      SmallVector<MachineInstr *, 4> ToRemove;

      for (auto &MI : MBB) {
        if (MI.getOpcode() != X86::ADD32rr)
          continue;

        Register DstReg = MI.getOperand(0).getReg();
        Register Src1Reg = MI.getOperand(1).getReg();
        Register Src2Reg = MI.getOperand(2).getReg();
        DebugLoc DL = MI.getDebugLoc();

        // Insert call sequence
        BuildMI(MBB, MI, DL, TII->get(X86::ADJCALLSTACKDOWN64))
            .addImm(0)
            .addImm(0);

        BuildMI(MBB, MI, DL, TII->get(X86::CALL64pcrel32))
            .addGlobalAddress(AddFunc)
            .addReg(Src1Reg, RegState::Implicit)
            .addReg(Src2Reg, RegState::Implicit)
            .addReg(DstReg, RegState::ImplicitDefine)
            .addRegMask(TRI->getCallPreservedMask(MF, F.getCallingConv()));

        BuildMI(MBB, MI, DL, TII->get(X86::ADJCALLSTACKUP64))
            .addImm(0)
            .addImm(0);

        ToRemove.push_back(&MI);
        Changed = true;
      }

      for (auto *MI : ToRemove)
        MI->eraseFromParent();
    }

    return Changed;
  }

  StringRef getPassName() const override { return "Replace ADD with CALL"; }
};

char AddToCallPass::ID = 0;

} // end anonymous namespace

static RegisterPass<AddToCallPass>
    X("add-to-call", "Replace ADD instructions with calls to add function",
      false, false);
