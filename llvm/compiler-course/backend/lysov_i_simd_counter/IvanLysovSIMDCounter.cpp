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

class InstSimdPass : public MachineFunctionPass {
public:
  static char ID;
  InstSimdPass() : MachineFunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    LLVMContext &Ctx = M.getContext();
    Type *Int64Ty = Type::getInt64Ty(Ctx);
    Constant *Zero = ConstantInt::get(Int64Ty, 0);

    GlobalVariable *CounterGV =
        cast<GlobalVariable>(M.getOrInsertGlobal("simd_counter", Int64Ty));
    CounterGV->setLinkage(GlobalValue::ExternalLinkage);
    CounterGV->setInitializer(Zero);
    CounterGV->setAlignment(MaybeAlign(8));
    return true;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineModuleInfo &MMI = MF.getMMI();
    const Module *Mod = MMI.getModule();
    const GlobalVariable *CounterGV = Mod->getGlobalVariable("simd_counter");
    if (!CounterGV)
      return false;

    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    const X86RegisterInfo &RI = TII->getRegisterInfo();
    const TargetRegisterClass *GPR64RC = RI.getRegClass(X86::GR64RegClassID);

    bool Changed = false;
    for (auto &MBB : MF) {
      for (auto MIIt = MBB.begin(), E = MBB.end(); MIIt != E; ++MIIt) {
        MachineInstr &MI = *MIIt;

        bool isSimd = false;
        for (const MachineOperand &MO : MI.operands()) {
          if (!MO.isReg())
            continue;
          Register R(MO.getReg());

          if (!R.isVirtual())
            continue;
          const TargetRegisterClass *RC = MRI.getRegClass(MO.getReg());
          unsigned RCID = RC->getID();
          if (RCID == X86::VR128RegClassID || RCID == X86::VR256RegClassID ||
              RCID == X86::VR512RegClassID) {
            isSimd = true;
            break;
          }
        }
        if (!isSimd)
          continue;

        DebugLoc DL = MI.getDebugLoc();
        auto InsertPt = std::next(MIIt);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::INC64m))
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)                      // scale (unused)
            .addReg(0, RegState::Implicit)  // index
            .addGlobalAddress(CounterGV, 0) // символ simd_counter

            .addReg(0, RegState::Implicit); // segment

        Changed = true;
      }
    }

    return Changed;
  }
};

char InstSimdPass::ID = 0;

} // namespace

static RegisterPass<InstSimdPass>
    X("inst-simd-x86", "Instrument SIMD instructions", false, false);
