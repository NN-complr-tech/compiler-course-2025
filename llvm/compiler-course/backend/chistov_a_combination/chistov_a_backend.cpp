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
#include <functional>

namespace {
class X86LogicOptPass : public llvm::MachineFunctionPass {
private:
  bool Changed = false;
  const llvm::X86InstrInfo *TII;
  const llvm::MachineRegisterInfo *MRI;

  using OptimizationPredicate = std::function<bool(llvm::MachineInstr &)>;
  using OptimizationTransform =
      std::function<void(llvm::MachineBasicBlock &, llvm::MachineInstr &,
                         llvm::SmallVectorImpl<llvm::MachineInstr *> &)>;

private:
  void applyOptimizations(llvm::MachineFunction &MF,
                          OptimizationPredicate shouldTransform,
                          OptimizationTransform performTransform) {
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> InstrsToRemove;
      for (auto &MI : MBB) {
        if (shouldTransform(MI)) {
          performTransform(MBB, MI, InstrsToRemove);
          Changed = true;
        }
      }
      for (auto *I : InstrsToRemove)
        I->eraseFromParent();
    }
  }

  bool isLogicOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case llvm::X86::ANDPSrr:
    case llvm::X86::ORPSrr:
    case llvm::X86::XORPSrr:
    case llvm::X86::PANDrr:
    case llvm::X86::PORrr:
    case llvm::X86::PXORrr:
    case llvm::X86::PANDNrr:
      return true;
    default:
      return false;
    }
  }

  unsigned getAVXOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case llvm::X86::ANDPSrr:
      return llvm::X86::VANDPSrr;
    case llvm::X86::ORPSrr:
      return llvm::X86::VORPSrr;
    case llvm::X86::XORPSrr:
      return llvm::X86::VXORPSrr;
    case llvm::X86::PANDrr:
      return llvm::X86::VPANDrr;
    case llvm::X86::PORrr:
      return llvm::X86::VPORrr;
    case llvm::X86::PXORrr:
      return llvm::X86::VPXORrr;
    case llvm::X86::PANDNrr:
      return llvm::X86::VPANDNrr;
    default:
      return 0;
    }
  }

  void replaceWithAVX(llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
                      llvm::SmallVectorImpl<llvm::MachineInstr *> &ToRemove) {
    unsigned NewOpc = getAVXOpcode(MI.getOpcode());
    if (!NewOpc)
      return;

    llvm::Register DestReg = MI.getOperand(0).getReg();
    llvm::Register SrcReg1 = MI.getOperand(1).getReg();
    llvm::Register SrcReg2 = MI.getOperand(2).getReg();

    llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(NewOpc), DestReg)
        .addReg(SrcReg1)
        .addReg(SrcReg2);
    ToRemove.push_back(&MI);
  }

  bool isZeroRegister(llvm::Register Reg) const {
    if (auto *DefInstr = MRI->getUniqueVRegDef(Reg)) {
      switch (DefInstr->getOpcode()) {
      case llvm::X86::PXORrr:
      case llvm::X86::XORPSrr:
      case llvm::X86::XORPDrr:
        auto &Op1 = DefInstr->getOperand(1);
        auto &Op2 = DefInstr->getOperand(2);
        return Op1.isReg() && Op2.isReg() && Op1.getReg() == Op2.getReg();
      }
    }
    return false;
  }

  bool
  handleZeroOperands(llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
                     unsigned Opc, llvm::Register DestReg,
                     llvm::Register SrcReg1, llvm::Register SrcReg2,
                     llvm::SmallVectorImpl<llvm::MachineInstr *> &ToRemove) {
    const bool Src1IsZero = isZeroRegister(SrcReg1);
    const bool Src2IsZero = isZeroRegister(SrcReg2);

    if (!Src1IsZero && !Src2IsZero)
      return false;

    switch (Opc) {
    case llvm::X86::PANDrr: {
      llvm::Register ZeroReg = Src1IsZero ? SrcReg1 : SrcReg2;
      llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(llvm::X86::VPXORrr),
                    DestReg)
          .addReg(ZeroReg)
          .addReg(ZeroReg);
      break;
    }
    case llvm::X86::PORrr: {
      llvm::Register NonZeroReg = Src1IsZero ? SrcReg2 : SrcReg1;
      llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                    TII->get(llvm::TargetOpcode::COPY), DestReg)
          .addReg(NonZeroReg);
      break;
    }
    case llvm::X86::PXORrr: {
      if (!Src1IsZero)
        return false;

      llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                    TII->get(llvm::TargetOpcode::COPY), DestReg)
          .addReg(SrcReg2);
      break;
    }
    default:
      return false;
    }

    ToRemove.push_back(&MI);
    return true;
  }

private:
  void optimizeLogicOperations(llvm::MachineFunction &MF) {
    auto shouldTransform = [&](llvm::MachineInstr &MI) {
      return isLogicOpcode(MI.getOpcode());
    };

    auto transformInstr =
        [&](llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
            llvm::SmallVectorImpl<llvm::MachineInstr *> &ToRemove) {
          const unsigned Opc = MI.getOpcode();
          const llvm::Register DestReg = MI.getOperand(0).getReg();
          const llvm::Register SrcReg1 = MI.getOperand(1).getReg();
          const llvm::Register SrcReg2 = MI.getOperand(2).getReg();

          if (handleZeroOperands(MBB, MI, Opc, DestReg, SrcReg1, SrcReg2,
                                 ToRemove)) {
            return;
          }

          replaceWithAVX(MBB, MI, ToRemove);
        };

    applyOptimizations(MF, shouldTransform, transformInstr);
  }

  void combineLogicOperations(llvm::MachineFunction &MF) {
    OptimizationPredicate shouldTransform = [&](llvm::MachineInstr &MI) {
      return isLogicOpcode(MI.getOpcode());
    };

    OptimizationTransform transformInstr =
        [&](llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
            llvm::SmallVectorImpl<llvm::MachineInstr *> &InstrsToRemove) {
          llvm::Register IntermediateReg = MI.getOperand(1).getReg();
          auto *FirstInstr = MRI->getUniqueVRegDef(IntermediateReg);
          if (!FirstInstr || !MRI->hasOneUse(IntermediateReg))
            return;

          unsigned FirstOpcode = FirstInstr->getOpcode();
          if (!isLogicOpcode(FirstOpcode))
            return;

          unsigned AVXFirstOp = getAVXOpcode(FirstOpcode);
          unsigned AVXSecondOp = getAVXOpcode(MI.getOpcode());
          if (!AVXFirstOp || !AVXSecondOp)
            return;

          replaceWithAVX(MBB, *FirstInstr, InstrsToRemove);
          replaceWithAVX(MBB, MI, InstrsToRemove);
        };

    applyOptimizations(MF, shouldTransform, transformInstr);
  }

public:
  static char ID;
  X86LogicOptPass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const auto *ST = &MF.getSubtarget<llvm::X86Subtarget>();
    TII = ST->getInstrInfo();
    MRI = &MF.getRegInfo();

    optimizeLogicOperations(MF);
    combineLogicOperations(MF);

    return Changed;
  }
};

char X86LogicOptPass::ID = 0;
} // namespace

static llvm::RegisterPass<X86LogicOptPass>
    X("x86-logic-opt", "X86 Logical Operations Optimization Pass", false,
      false);