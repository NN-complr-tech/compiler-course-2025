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

static bool touchesVectorReg(const MachineInstr &MI,
                             const MachineRegisterInfo &MRI) {
  for (const MachineOperand &Op : MI.operands()) {
    if (!Op.isReg())
      continue;
    unsigned Reg = Op.getReg();
    if (!Register(Reg).isVirtual())
      continue;
    const TargetRegisterClass *RC = MRI.getRegClass(Reg);
    const unsigned RCID = RC->getID();
    if (RCID == X86::VR128RegClassID || RCID == X86::VR256RegClassID ||
        RCID == X86::VR512RegClassID)
      return true;
  }
  return false;
}

class InstrumentSimdPass : public MachineFunctionPass {
public:
  static char ID;
  InstrumentSimdPass() : MachineFunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    LLVMContext &Ctx = M.getContext();
    Type *I64 = Type::getInt64Ty(Ctx);
    Constant *Zero = ConstantInt::get(I64, 0);
    auto *GV = new GlobalVariable(M, I64, false, GlobalValue::ExternalLinkage,
                                  Zero, "simd_counter");
    GV->setAlignment(MaybeAlign(8));
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
      for (auto It = MBB.begin(), End = MBB.end(); It != End; ++It) {
        MachineInstr &MI = *It;
        if (!touchesVectorReg(MI, MRI))
          continue;

        DebugLoc DL = MI.getDebugLoc();
        auto InsertPos = std::next(It);
        Register CtrVReg = MRI.createVirtualRegister(GPR64RC);

        BuildMI(MBB, InsertPos, DL, TII->get(X86::MOV64rm), CtrVReg)
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)
            .addReg(0, RegState::Implicit)
            .addGlobalAddress(CounterGV, 0)
            .addReg(0, RegState::Implicit);

        BuildMI(MBB, InsertPos, DL, TII->get(X86::ADD64ri32), CtrVReg)
            .addReg(CtrVReg)
            .addImm(1);

        BuildMI(MBB, InsertPos, DL, TII->get(X86::MOV64mr))
            .addReg(X86::RIP, RegState::Implicit)
            .addImm(1)
            .addReg(0, RegState::Implicit)
            .addGlobalAddress(CounterGV, 0)
            .addReg(CtrVReg);

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
