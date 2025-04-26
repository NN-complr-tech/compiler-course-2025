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

#define DEBUG_TYPE "instrument-simd"

using namespace llvm;

namespace {

class InstrumentSimdPass : public MachineFunctionPass {
public:
  static char ID;
  InstrumentSimdPass() : MachineFunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    LLVMContext &Ctx = M.getContext();
    Type *Int64Ty = Type::getInt64Ty(Ctx);
    Constant *Zero = ConstantInt::get(Int64Ty, 0);

    GlobalVariable *CounterGV =
        new GlobalVariable(M, Int64Ty, /*isConstant=*/false,
                           GlobalValue::ExternalLinkage, Zero, "simd_counter");
    CounterGV->setAlignment(MaybeAlign(8));
    return true;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineModuleInfo &MMI = MF.getMMI();
    const Module *Mod = MMI.getModule();
    const GlobalVariable *CounterGV = Mod->getGlobalVariable("simd_counter");
    if (!CounterGV)
      return false; // если по какой-то причине нет — ничего не делаем

    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    const X86RegisterInfo &RI = TII->getRegisterInfo();
    const TargetRegisterClass *GPR64RC = RI.getRegClass(X86::GR64RegClassID);

    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto MI_it = MBB.begin(), E = MBB.end(); MI_it != E; ++MI_it) {
        MachineInstr &MI = *MI_it;
        bool isSimd = false;
        for (const MachineOperand &MO : MI.operands()) {
          if (!MO.isReg())
            continue;
          unsigned Reg = MO.getReg();
          Register R(Reg);
          if (!R.isVirtual())
            continue;
          const TargetRegisterClass *RC = MRI.getRegClass(Reg);
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
        auto InsertPt = std::next(MI_it);

        Register CounterReg = MRI.createVirtualRegister(GPR64RC);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::MOV64rm), CounterReg)
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)
            .addReg(0, RegState::Implicit)
            .addGlobalAddress(CounterGV, 0)
            .addReg(0, RegState::Implicit);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::ADD64ri32), CounterReg)
            .addReg(CounterReg)
            .addImm(1);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::MOV64mr))
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)
            .addReg(0, RegState::Implicit)
            .addGlobalAddress(CounterGV, 0)
            .addReg(CounterReg);

        Changed = true;
      }
    }

    return Changed;
  }
};

char InstrumentSimdPass::ID = 0;

} // namespace

static llvm::RegisterPass<InstrumentSimdPass>
    X("instrument-simd-x86", "Instrument SIMD instructions", false, false);
