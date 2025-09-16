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

#define DEBUG_TYPE "simd-instrument"

using namespace llvm;

namespace {

  class SimdCounterPass : public MachineFunctionPass {
  public:
    static char ID;
    SimdCounterPass() : MachineFunctionPass(ID) {}

    bool doInitialization(Module& M) override {
      LLVMContext& C = M.getContext();
      Type* I64 = Type::getInt64Ty(C);
      Constant* InitZero = ConstantInt::get(I64, 0);

      auto* GV = new GlobalVariable(
        M, I64, false,
        GlobalValue::ExternalLinkage, InitZero, "simd_counter");

      GV->setAlignment(MaybeAlign(8));
      return true;
    }

    bool runOnMachineFunction(MachineFunction& MF) override {
      MachineModuleInfo& MMI = MF.getMMI();
      const Module* Mod = MMI.getModule();
      const GlobalVariable* GV = Mod->getGlobalVariable("simd_counter");
      if (!GV)
        return false;

      const X86Subtarget& SubT = MF.getSubtarget<X86Subtarget>();
      const X86InstrInfo* TII = SubT.getInstrInfo();
      MachineRegisterInfo& MRI = MF.getRegInfo();
      const X86RegisterInfo& RI = TII->getRegisterInfo();
      const TargetRegisterClass* RC64 = RI.getRegClass(X86::GR64RegClassID);

      bool Changed = false;

      for (auto& BB : MF) {
        for (auto It = BB.begin(); It != BB.end(); ++It) {
          MachineInstr& Inst = *It;

          bool HasSimd = false;
          for (const MachineOperand& Op : Inst.operands()) {
            if (!Op.isReg())
              continue;

            unsigned Reg = Op.getReg();
            Register R(Reg);
            if (!R.isVirtual())
              continue;

            const TargetRegisterClass* RC = MRI.getRegClass(Reg);
            unsigned ClassID = RC->getID();
            if (ClassID == X86::VR128RegClassID || ClassID == X86::VR256RegClassID ||
              ClassID == X86::VR512RegClassID) {
              HasSimd = true;
              break;
            }
          }

          if (!HasSimd)
            continue;

          DebugLoc DL = Inst.getDebugLoc();
          auto InsertHere = std::next(It);

          Register CReg = MRI.createVirtualRegister(RC64);

          BuildMI(BB, InsertHere, DL, TII->get(X86::MOV64rm), CReg)
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)
            .addReg(0, RegState::Implicit)
            .addGlobalAddress(GV, 0)
            .addReg(0, RegState::Implicit);

          BuildMI(BB, InsertHere, DL, TII->get(X86::ADD64ri32), CReg)
            .addReg(CReg)
            .addImm(1);

          BuildMI(BB, InsertHere, DL, TII->get(X86::MOV64mr))
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)
            .addReg(0, RegState::Implicit)
            .addGlobalAddress(GV, 0)
            .addReg(CReg);

          Changed = true;
        }
      }

      return Changed;
    }
  };

  char SimdCounterPass::ID = 0;

} // namespace

static llvm::RegisterPass<SimdCounterPass>
X("simd-counter-pass", "Count SIMD instructions", false, false);
