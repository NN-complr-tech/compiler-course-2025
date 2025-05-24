#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace {

class SimdInstrCounterPass : public MachineFunctionPass {
public:
  static char ID;
  SimdInstrCounterPass() : MachineFunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    LLVMContext &Ctx = M.getContext();
    Type *Int64Type = Type::getInt64Ty(Ctx);
    Constant *InitialValue = ConstantInt::get(Int64Type, 0);

    auto *GlobalCounter =
        cast<GlobalVariable>(M.getOrInsertGlobal("simd_counter", Int64Type));
    GlobalCounter->setInitializer(InitialValue);
    GlobalCounter->setLinkage(GlobalValue::ExternalLinkage);
    GlobalCounter->setAlignment(Align(8));

    return true;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const Module *M = MF.getMMI().getModule();
    const GlobalVariable *CounterVar = M->getGlobalVariable("simd_counter");
    if (!CounterVar)
      return false;

    const X86Subtarget &Subtarget = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *InstrInfo = Subtarget.getInstrInfo();
    MachineRegisterInfo &RegInfo = MF.getRegInfo();

    bool Modified = false;

    for (auto &Block : MF) {
      unsigned VectorInstrs = 0;

      for (auto &Instr : Block) {
        for (const auto &Op : Instr.operands()) {
          if (Op.isReg()) {
            Register Reg = Op.getReg();
            if (Reg.isVirtual()) {
              const TargetRegisterInfo *TRI =
                  MF.getSubtarget().getRegisterInfo();
              const RegisterBankInfo *RBI = MF.getSubtarget().getRegBankInfo();

              if (!RBI)
                continue;

              const RegisterBank *RB = RBI->getRegBank(Reg, RegInfo, *TRI);
              if (RB && RB->getID() == X86::VECRRegBankID) {
                const TargetRegisterClass *RC = RegInfo.getRegClass(Reg);
                unsigned RegSizeBits = RBI->getSizeInBits(Reg, RegInfo, *TRI);
                if (RegSizeBits > 64 &&
                    (RC == &X86::VR128RegClass || RC == &X86::VR256RegClass ||
                     RC == &X86::VR512RegClass)) {
                  ++VectorInstrs;
                  break;
                }
              }
            }
          }
        }
      }

      if (VectorInstrs == 0)
        continue;

      auto InsertPos = Block.getFirstTerminator();
      DebugLoc Loc;
      if (InsertPos != Block.end())
        Loc = InsertPos->getDebugLoc();

      BuildMI(Block, InsertPos, Loc, InstrInfo->get(X86::ADD64mi32))
          .addReg(X86::RIP, RegState::Implicit)
          .addImm(1)
          .addReg(0, RegState::Implicit)
          .addGlobalAddress(CounterVar)
          .addReg(0, RegState::Implicit)
          .addImm(VectorInstrs);

      Modified = true;
    }

    return Modified;
  }
};

char SimdInstrCounterPass::ID = 0;

} // namespace

static RegisterPass<SimdInstrCounterPass>
    X("simd-instr-counter", "Counts SIMD instructions and instruments counter",
      false, false);