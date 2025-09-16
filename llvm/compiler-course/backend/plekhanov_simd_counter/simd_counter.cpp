#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

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

    if (M.getGlobalVariable("simd_counter")) {
      return false;
    }

    Constant *Zero = ConstantInt::get(Int64Ty, 0);
    GlobalVariable *CounterGV = new GlobalVariable(
        M, Int64Ty, false, GlobalValue::ExternalLinkage, Zero, "simd_counter");
    CounterGV->setAlignment(MaybeAlign(8));
    return true;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineModuleInfo &MMI = MF.getMMI();
    const Module *Mod = MMI.getModule();
    const GlobalVariable *CounterGV = Mod->getGlobalVariable("simd_counter");

    if (!CounterGV) {
      errs() << "InstrumentSimdPass: simd_counter global variable not found.\n";
      return false;
    }

    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();

    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto MI_it = MBB.begin(), E = MBB.end(); MI_it != E; ++MI_it) {
        MachineInstr &MI = *MI_it;
        bool isSimd = false;

        for (const MachineOperand &MO : MI.operands()) {
          if (!MO.isReg() || !MO.getReg())
            continue;

          unsigned Reg = MO.getReg();
          if (!Register::isVirtualRegister(Reg))
            continue;

          const TargetRegisterClass *RC = MRI.getRegClass(Reg);
          if (RC) {
            unsigned RCID = RC->getID();
            if (RCID == X86::VR128RegClassID || RCID == X86::VR256RegClassID ||
                RCID == X86::VR512RegClassID) {
              isSimd = true;
              break;
            }
          }
        }

        if (!isSimd)
          continue;

        Changed = true;
        DebugLoc DL = MI.getDebugLoc();

        MachineInstrBuilder MIB =
            BuildMI(MBB, MI_it, DL, TII->get(X86::ADD64mi8));

        MIB.addReg(X86::RIP, RegState::Implicit);
        MIB.addImm(1);
        MIB.addReg(0, RegState::Implicit);
        MIB.addGlobalAddress(CounterGV, 0);
        MIB.addReg(0, RegState::Implicit);
        MIB.addImm(1);

        MachineMemOperand *MMO = MF.getMachineMemOperand(
            MachinePointerInfo(CounterGV),
            MachineMemOperand::MOLoad | MachineMemOperand::MOStore |
                MachineMemOperand::MOVolatile,
            8, Align(8));
        MIB.addMemOperand(MMO);
      }
    }
    return Changed;
  }
};

char InstrumentSimdPass::ID = 0;

} // namespace

static llvm::RegisterPass<InstrumentSimdPass>
    X("simd-pass", "Instrument SIMD instructions", false, false);
