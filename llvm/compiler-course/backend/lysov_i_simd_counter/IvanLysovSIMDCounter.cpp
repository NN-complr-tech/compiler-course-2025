#include "X86.h"
#include "X86InstrInfo.h"
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

class InstSimdPass : public MachineFunctionPass {
public:
  static char ID;
  InstSimdPass() : MachineFunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    LLVMContext &Ctx = M.getContext();
    Type *Int64Ty = Type::getInt64Ty(Ctx);
    Constant *Zero = ConstantInt::get(Int64Ty, 0);

    auto *GV =
        cast<GlobalVariable>(M.getOrInsertGlobal("simd_counter", Int64Ty));
    GV->setInitializer(Zero);
    GV->setLinkage(GlobalValue::ExternalLinkage);
    GV->setAlignment(MaybeAlign(8));
    return true;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const Module *Mod = MF.getMMI().getModule();
    const GlobalVariable *CounterGV = Mod->getGlobalVariable("simd_counter");
    if (!CounterGV)
      return false;

    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();

    bool Changed = false;

    for (MachineBasicBlock &MBB : MF) {
      unsigned SimdCount = 0;

      for (MachineInstr &MI : MBB) {
        for (const MachineOperand &MO : MI.operands()) {
          if (!MO.isReg())
            continue;
          Register R = MO.getReg();
          if (!R.isVirtual())
            continue;
          const TargetRegisterClass *RC = MRI.getRegClass(R);
          unsigned RCID = RC->getID();
          if (RCID == X86::VR128RegClassID || RCID == X86::VR256RegClassID ||
              RCID == X86::VR512RegClassID) {
            ++SimdCount;
            break;
          }
        }
      }

      if (SimdCount == 0)
        continue;

      MachineBasicBlock::iterator InsertPt = MBB.getFirstTerminator();
      DebugLoc DL;
      if (InsertPt != MBB.end())
        DL = InsertPt->getDebugLoc();

      BuildMI(MBB, InsertPt, DL, TII->get(X86::ADD64mi32))
          .addReg(X86::RIP, RegState::Implicit)
          .addImm(1)
          .addReg(0, RegState::Implicit)
          .addGlobalAddress(CounterGV, 0)
          .addReg(0, RegState::Implicit)
          .addImm(SimdCount);

      Changed = true;
    }
    return Changed;
  }
};

char InstSimdPass::ID = 0;

} // namespace

static RegisterPass<InstSimdPass>
    X("inst-simd-x86", "Instrument SIMD instructions", false, false);
