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
#include "llvm/IR/Module.h"  
#include "llvm/IR/Function.h"

#define DEBUG_TYPE "add-to-call"

namespace {

class AddToCallPass : public llvm::MachineFunctionPass {
public:
  static char ID;
  AddToCallPass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &ST = MF.getSubtarget<llvm::X86Subtarget>();
    const llvm::X86InstrInfo *TII = ST.getInstrInfo();
    bool Changed = false;

    // Ищем функцию add в модуле
    llvm::Function *AddFunc = MF.getFunction().getParent()->getFunction("add");
    if (!AddFunc)
      return false;

    // Проверяем сигнатуру функции add (i32, i32) -> i32
    if (AddFunc->getFunctionType()->getNumParams() != 2 ||
        !AddFunc->getFunctionType()->getReturnType()->isIntegerTy(32) ||
        !AddFunc->getFunctionType()->getParamType(0)->isIntegerTy(32) ||
        !AddFunc->getFunctionType()->getParamType(1)->isIntegerTy(32)) {
      return false;
    }

    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> ToErase;

      for (auto &MI : MBB) {
        // Пропускаем инструкции, которые не являются ADD
        if (MI.getOpcode() != llvm::X86::ADD32rr)
          continue;

        // Получаем операнды
        llvm::Register Dst = MI.getOperand(0).getReg();
        llvm::Register Src1 = MI.getOperand(1).getReg();
        llvm::Register Src2 = MI.getOperand(2).getReg();
        llvm::DebugLoc DL = MI.getDebugLoc();

        // Создаем вызов функции add вместо инструкции ADD
        llvm::BuildMI(MBB, MI, DL, TII->get(llvm::X86::CALL32r))
            .addGlobalAddress(AddFunc)
            .addReg(Src1, llvm::RegState::Implicit)
            .addReg(Src2, llvm::RegState::Implicit)
            .addReg(Dst, llvm::RegState::Implicit | llvm::RegState::Define);

        ToErase.push_back(&MI);
        Changed = true;
      }

      // Удаляем заменённые инструкции
      for (auto *MI : ToErase)
        MI->eraseFromParent();
    }

    return Changed;
  }
};

char AddToCallPass::ID = 0;

} // end anonymous namespace

static llvm::RegisterPass<AddToCallPass>
    X("add-to-call", "Replaces ADD instructions with calls to add function",
      false, false);
