#include "X86.h"
#include "X86InstrInfo.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

namespace {
class CounterPass : public MachineFunctionPass {
public:
  static char ID;
  CounterPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
  bool isSIMDInstruction(const Module *module, const MachineFunction &func,
                         const MachineInstr &instr);
};

char CounterPass::ID = 0;

bool CounterPass::runOnMachineFunction(MachineFunction &func) {
  const TargetInstrInfo *info = func.getSubtarget().getInstrInfo();
  Module *module = func.getFunction().getParent();
  bool isChanged = false;

  GlobalVariable *counter = module->getGlobalVariable("counter");
  if (!counter) {
    counter = new GlobalVariable(
        *module, Type::getInt64Ty(module->getContext()), false,
        GlobalValue::ExternalLinkage,
        ConstantInt::get(Type::getInt64Ty(module->getContext()), 0), "counter");
    counter->setAlignment(MaybeAlign(8));
  }

  for (MachineBasicBlock &block : func) {
    for (MachineInstr &instr : block) {
      if (isSIMDInstruction(module, func, instr)) {
        auto pos = std::next(instr.getIterator());
        DebugLoc loc = instr.getDebugLoc();

        const MCInstrDesc &movTo = info->get(llvm::X86::MOV64rm);
        const MCInstrDesc &add = info->get(llvm::X86::ADD64ri32);
        const MCInstrDesc &movFrom = info->get(llvm::X86::MOV64mr);

        const TargetRegisterInfo *regInfo =
            func.getSubtarget().getRegisterInfo();
        const TargetRegisterClass *GR64RC =
            regInfo->getRegClass(X86::GR64RegClassID);
        Register tmpReg = func.getRegInfo().createVirtualRegister(GR64RC);

        BuildMI(block, pos, loc, movTo, tmpReg)
            .addGlobalAddress(counter)
            .addImm(1)
            .addReg(0)
            .addImm(0)
            .addReg(0);

        BuildMI(block, pos, loc, add, tmpReg).addReg(tmpReg).addImm(1);

        BuildMI(block, pos, loc, movFrom)
            .addGlobalAddress(counter)
            .addImm(1)
            .addReg(0)
            .addImm(0)
            .addReg(0)
            .addReg(tmpReg);

        isChanged = true;
      }
    }
  }

  return isChanged;
}

bool CounterPass ::isSIMDInstruction(const Module *module,
                                     const MachineFunction &func,
                                     const MachineInstr &instr) {
  const MCInstrDesc &desc = instr.getDesc();
  if (desc.isTerminator() || desc.isReturn() || desc.isBranch() ||
      desc.isCall() || desc.isPseudo()) {
    return false;
  }
  for (const MachineOperand &operand : instr.operands()) {
    if (operand.isReg()) {
      Register reg = operand.getReg();
      if (reg.isVirtual()) {
        unsigned id = func.getRegInfo().getRegClass(reg)->getID();
        if (id == X86::VR128RegClassID || id == X86::VR256RegClassID ||
            id == X86::VR512RegClassID) {
          return true;
        }
      }
    }
  }
  return false;
}
} // namespace

static RegisterPass<CounterPass>
    X("counter-pass-x86",
      "This pass for each SIMD instruction inserts code that increments the "
      "count of executed vector functions",
      false, false);
