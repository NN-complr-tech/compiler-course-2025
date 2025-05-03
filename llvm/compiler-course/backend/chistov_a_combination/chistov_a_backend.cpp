#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include <functional>

namespace {
class X86LogicOptPass : public llvm::MachineFunctionPass {
private:
  bool Changed = false;
  const llvm::X86InstrInfo *TII;
  const llvm::MachineRegisterInfo *MRI;

  llvm::DenseMap<unsigned, unsigned> LogicToAVX = {
      {llvm::X86::ANDPSrr, llvm::X86::VANDPSrr},
      {llvm::X86::ORPSrr, llvm::X86::VORPSrr},
      {llvm::X86::XORPSrr, llvm::X86::VXORPSrr},
      {llvm::X86::PANDrr, llvm::X86::VPANDrr},
      {llvm::X86::PORrr, llvm::X86::VPORrr},
      {llvm::X86::PXORrr, llvm::X86::VPXORrr},
      {llvm::X86::PANDNrr, llvm::X86::VPANDNrr},
  };

  using OptimizationPredicate = std::function<bool(llvm::MachineInstr &)>;
  using OptimizationTransform =
      std::function<void(llvm::MachineBasicBlock &, llvm::MachineInstr &,
                         llvm::SmallVectorImpl<llvm::MachineInstr *> &)>;

private: // auxiliary functions
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
    return LogicToAVX.contains(Opcode);
  }

  unsigned getAVXOpcode(unsigned Opcode) const {
    auto It = LogicToAVX.find(Opcode);
    return It != LogicToAVX.end() ? It->second : 0;
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
    Changed = true;
  }

private: // optimizing passes
  void optimizeLogicOperations(llvm::MachineFunction &MF) {
    OptimizationPredicate shouldTransform = [&](llvm::MachineInstr &MI) {
      return isLogicOpcode(MI.getOpcode());
    };

    OptimizationTransform transformInstr =
        [&](llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
            llvm::SmallVectorImpl<llvm::MachineInstr *> &ToRemove) {
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
    Changed = false;
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
