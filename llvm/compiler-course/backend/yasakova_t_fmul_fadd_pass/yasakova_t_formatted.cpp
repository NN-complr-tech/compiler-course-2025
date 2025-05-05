#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include <unordered_map>

namespace llvm {

class X86LogicalOpsEnhancer : public MachineFunctionPass {
public:
  static char ID;

  X86LogicalOpsEnhancer() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "X86 Logical Operations Enhancement Pass";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (!initializeOptimization(MF))
      return false;

    processLogicInstructions(MF);
    combineAdjacentLogicOps(MF);

    return OptimizationModified;
  }

private:
  bool OptimizationModified = false;
  const X86InstrInfo *InstructionDetails = nullptr;
  const MachineRegisterInfo *RegisterDetails = nullptr;

  struct InstructionConversion {
    unsigned LegacyOp;
    unsigned EnhancedOp;
  };

  const SmallVector<InstructionConversion, 8> ConversionTable = {
      {X86::ANDPSrr, X86::VANDPSrr}, {X86::ORPSrr, X86::VORPSrr},
      {X86::XORPSrr, X86::VXORPSrr}, {X86::PANDrr, X86::VPANDrr},
      {X86::PORrr, X86::VPORrr},     {X86::PXORrr, X86::VPXORrr},
      {X86::PANDNrr, X86::VPANDNrr},
  };

  bool initializeOptimization(MachineFunction &MF) {
    const auto &ST = MF.getSubtarget<X86Subtarget>();
    InstructionDetails = ST.getInstrInfo();
    RegisterDetails = &MF.getRegInfo();
    return InstructionDetails && RegisterDetails;
  }

  bool shouldConvertInstruction(unsigned Opcode) const {
    for (const auto &Entry : ConversionTable) {
      if (Entry.LegacyOp == Opcode)
        return true;
    }
    return false;
  }

  unsigned getEnhancedVariant(unsigned Opcode) const {
    for (const auto &Entry : ConversionTable) {
      if (Entry.LegacyOp == Opcode)
        return Entry.EnhancedOp;
    }
    return 0;
  }

  void transformToEnhancedVariant(MachineBasicBlock &MBB,
                                  MachineInstr &OriginalInstr,
                                  SmallVectorImpl<MachineInstr *> &ForRemoval) {
    unsigned NewOpcode = getEnhancedVariant(OriginalInstr.getOpcode());
    if (!NewOpcode)
      return;

    Register Destination = OriginalInstr.getOperand(0).getReg();
    Register Source1 = OriginalInstr.getOperand(1).getReg();
    Register Source2 = OriginalInstr.getOperand(2).getReg();

    BuildMI(MBB, OriginalInstr, OriginalInstr.getDebugLoc(),
            InstructionDetails->get(NewOpcode), Destination)
        .addReg(Source1)
        .addReg(Source2);

    ForRemoval.push_back(&OriginalInstr);
    OptimizationModified = true;
  }

  void processInstructions(
      MachineFunction &MF,
      function_ref<bool(MachineInstr &)> Filter,
      function_ref<void(MachineBasicBlock &, MachineInstr &,
                        SmallVectorImpl<MachineInstr *> &)> Transformer) {
    for (auto &MBB : MF) {
      SmallVector<MachineInstr *, 4> InstructionsToRemove;

      for (auto &MI : MBB) {
        if (Filter(MI)) {
          Transformer(MBB, MI, InstructionsToRemove);
        }
      }

      for (auto *Instr : InstructionsToRemove) {
        Instr->eraseFromParent();
      }
    }
  }

  void processLogicInstructions(MachineFunction &MF) {
    auto InstructionFilter = [this](MachineInstr &MI) {
      return shouldConvertInstruction(MI.getOpcode());
    };

    auto Transformation = [this](MachineBasicBlock &MBB, MachineInstr &MI,
                                 SmallVectorImpl<MachineInstr *> &ToRemove) {
      transformToEnhancedVariant(MBB, MI, ToRemove);
    };

    processInstructions(MF, InstructionFilter, Transformation);
  }

  void combineAdjacentLogicOps(MachineFunction &MF) {
    auto InstructionFilter = [this](MachineInstr &MI) {
      return shouldConvertInstruction(MI.getOpcode());
    };

    auto Transformation = [this](MachineBasicBlock &MBB, MachineInstr &MI,
                                 SmallVectorImpl<MachineInstr *> &ToRemove) {
      Register IntermediateReg = MI.getOperand(1).getReg();
      MachineInstr *FirstInstr =
          RegisterDetails->getUniqueVRegDef(IntermediateReg);

      if (!FirstInstr || !RegisterDetails->hasOneUse(IntermediateReg))
        return;

      if (!shouldConvertInstruction(FirstInstr->getOpcode()))
        return;

      transformToEnhancedVariant(MBB, *FirstInstr, ToRemove);
      transformToEnhancedVariant(MBB, MI, ToRemove);
    };

    processInstructions(MF, InstructionFilter, Transformation);
  }
};

char X86LogicalOpsEnhancer::ID = 0;

static RegisterPass<X86LogicalOpsEnhancer>
    Y("x86-logic-enhancer", "Enhances X86 Logical Operations", false, false);

} // namespace llvm
