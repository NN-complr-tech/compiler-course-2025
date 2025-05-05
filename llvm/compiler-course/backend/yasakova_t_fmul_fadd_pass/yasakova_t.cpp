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
  bool Modified = false;
  const llvm::X86InstrInfo *InstrInfo;
  const llvm::MachineRegisterInfo *MRegisterInfo;

  llvm::DenseMap<unsigned, unsigned> LogicOpAVXMapping = {
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

private:
  void processOptimizations(llvm::MachineFunction &MF,
                            OptimizationPredicate shouldTransform,
                            OptimizationTransform performTransform) {
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> InstrsToRemove;
      for (auto &MI : MBB) {
        if (shouldTransform(MI)) {
          performTransform(MBB, MI, InstrsToRemove);
          Modified = true;
        }
      }
      for (auto *I : InstrsToRemove)
        I->eraseFromParent();
    }
  }

  bool isLogicOpcode(unsigned Opcode) const {
    return LogicOpAVXMapping.contains(Opcode);
  }

  unsigned getAVXOpcode(unsigned Opcode) const {
    auto It = LogicOpAVXMapping.find(Opcode);
    return It != LogicOpAVXMapping.end() ? It->second : 0;
  }

  void convertToAVX(llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
                    llvm::SmallVectorImpl<llvm::MachineInstr *> &ToRemove) {
    unsigned NewOpc = getAVXOpcode(MI.getOpcode());
    if (!NewOpc)
      return;

    llvm::Register DestReg = MI.getOperand(0).getReg();
    llvm::Register SrcReg1 = MI.getOperand(1).getReg();
    llvm::Register SrcReg2 = MI.getOperand(2).getReg();

    llvm::BuildMI(MBB, MI, MI.getDebugLoc(), InstrInfo->get(NewOpc), DestReg)
        .addReg(SrcReg1)
        .addReg(SrcReg2);
    ToRemove.push_back(&MI);
    Modified = true;
  }

private: // optimizing passes
  void processLogicOps(llvm::MachineFunction &MF) {
    OptimizationPredicate shouldTransform = [&](llvm::MachineInstr &MI) {
      return isLogicOpcode(MI.getOpcode());
    };

    OptimizationTransform transformInstr =
        [&](llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
            llvm::SmallVectorImpl<llvm::MachineInstr *> &ToRemove) {
          convertToAVX(MBB, MI, ToRemove);
        };

    processOptimizations(MF, shouldTransform, transformInstr);
  }

  void mergeLogicOps(llvm::MachineFunction &MF) {
    OptimizationPredicate shouldTransform = [&](llvm::MachineInstr &MI) {
      return isLogicOpcode(MI.getOpcode());
    };

    OptimizationTransform transformInstr =
        [&](llvm::MachineBasicBlock &MBB, llvm::MachineInstr &MI,
            llvm::SmallVectorImpl<llvm::MachineInstr *> &InstrsToRemove) {
          llvm::Register IntermediateReg = MI.getOperand(1).getReg();
          auto *FirstInstr = MRegisterInfo->getUniqueVRegDef(IntermediateReg);
          if (!FirstInstr || !MRegisterInfo->hasOneUse(IntermediateReg))
            return;

          unsigned FirstOpcode = FirstInstr->getOpcode();
          if (!isLogicOpcode(FirstOpcode))
            return;

          unsigned AVXFirstOp = getAVXOpcode(FirstOpcode);
          unsigned AVXSecondOp = getAVXOpcode(MI.getOpcode());
          if (!AVXFirstOp || !AVXSecondOp)
            return;

          convertToAVX(MBB, *FirstInstr, InstrsToRemove);
          convertToAVX(MBB, MI, InstrsToRemove);
        };

    processOptimizations(MF, shouldTransform, transformInstr);
  }

public:
  static char ID;
  X86LogicOptPass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    Modified = false;
    const auto *ST = &MF.getSubtarget<llvm::X86Subtarget>();
    InstrInfo = ST->getInstrInfo();
    MRegisterInfo = &MF.getRegInfo();

    processLogicOps(MF);
    mergeLogicOps(MF);

    return Modified;
  }
};

char X86LogicOptPass::ID = 0;
} // namespace

static llvm::RegisterPass<X86LogicOptPass>
    X("x86-logic-opt", "X86 Logical Operations Optimization Pass", false,
      false);