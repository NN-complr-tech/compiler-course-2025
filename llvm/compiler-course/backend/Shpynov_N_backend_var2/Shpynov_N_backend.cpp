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
class SIMDInstructionCounterPass : public MachineFunctionPass {
public:
  static char ID;
  SIMDInstructionCounterPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
  bool isSIMDInstruction(const MachineFunction &func,
                         const MachineInstr &instr);
};

bool SIMDInstructionCounterPass::isSIMDInstruction(const MachineFunction &func,
                                                   const MachineInstr &instr) {
  const MCInstrDesc &desc = instr.getDesc();

  if (desc.isTerminator() || desc.isReturn() || desc.isBranch() ||
      desc.isCall() || desc.isPseudo()) {
    return false;
  }

  bool hasVectorReg = false;

  for (const MachineOperand &operand : instr.operands()) {
    if (!operand.isReg())
      continue;
    Register reg = operand.getReg();
    if (reg == 0)
      continue;
    unsigned id = func.getRegInfo().getRegClass(reg)->getID();
    if (id == X86::VR128RegClassID || id == X86::VR256RegClassID ||
        id == X86::VR512RegClassID) {
      hasVectorReg = true;
      break;
    }
  }
  return hasVectorReg &&
         !(instr.isPHI() || instr.isCopy() || instr.isMoveImmediate());
}
char SIMDInstructionCounterPass::ID = 0;

bool SIMDInstructionCounterPass::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;
  Module *M = MF.getFunction().getParent();
  GlobalVariable *CounterVar = M->getNamedGlobal("SIMD_instruction_counter");
  if (!CounterVar) {
    CounterVar = new GlobalVariable(
        *M, Type::getInt64Ty(M->getContext()), false,
        GlobalValue::ExternalLinkage,
        ConstantInt::get(Type::getInt64Ty(M->getContext()), 0),
        "SIMD_instruction_counter");
  }

  for (auto &MBB : MF) {
    for (auto MI = MBB.begin(), E = MBB.end(); MI != E; ++MI) {
      MachineInstr &Instr = *MI;
      const TargetRegisterInfo *regInfo = MF.getSubtarget().getRegisterInfo();

      if (isSIMDInstruction(MF, Instr)) {
        const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
        DebugLoc DL = Instr.getDebugLoc();
        Register reg = MF.getRegInfo().createVirtualRegister(
            regInfo->getRegClass(X86::GR64RegClassID));

        BuildMI(MBB, Instr, DL, TII->get(X86::MOV64rm), reg)
            .addGlobalAddress(CounterVar)
            .addImm(1)
            .addReg(0)
            .addImm(0)
            .addReg(0);

        BuildMI(MBB, Instr, DL, TII->get(X86::ADD64ri32), reg)
            .addReg(reg)
            .addImm(1);

        BuildMI(MBB, Instr, DL, TII->get(X86::MOV64mr))
            .addGlobalAddress(CounterVar)
            .addImm(1)
            .addReg(0)
            .addImm(0)
            .addReg(0)
            .addReg(reg);
        Modified = true;
      }
    }
  }
  return Modified;
}
} // namespace

static RegisterPass<SIMDInstructionCounterPass>
    X("simd-instrustion-counter-pass-x86", "SIMD Instruction Counter Pass",
      false, false);