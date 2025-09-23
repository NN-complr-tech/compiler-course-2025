#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {

class FloatingPointFusionPass : public MachineFunctionPass {
public:
  static char Identifier;

  FloatingPointFusionPass() : MachineFunctionPass(Identifier) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const X86InstrInfo *InstructionInfo = nullptr;

  bool processBasicBlock(MachineBasicBlock &Block);
  std::optional<unsigned>
  determineFusedOpcode(unsigned multiplicationOpcode, unsigned arithmeticOpcode,
                       bool isOperandOrderReversed) const;
  bool validateOperandPattern(const MachineInstr &multiplicationInstruction,
                              const MachineInstr &arithmeticInstruction,
                              bool &isOperandOrderReversed) const;
};

char FloatingPointFusionPass::Identifier = 0;

bool FloatingPointFusionPass::runOnMachineFunction(MachineFunction &MF) {
  const auto &targetSubsystem = MF.getSubtarget<X86Subtarget>();
  if (!targetSubsystem.hasFMA())
    return false;

  InstructionInfo = targetSubsystem.getInstrInfo();
  bool functionModified = false;

  for (auto &basicBlock : MF)
    functionModified |= processBasicBlock(basicBlock);

  return functionModified;
}

bool FloatingPointFusionPass::processBasicBlock(MachineBasicBlock &Block) {
  SmallVector<MachineInstr *, 4> instructionsToRemove;
  auto &registerInformation = Block.getParent()->getRegInfo();
  bool blockModified = false;

  for (auto currentInstruction = Block.begin(), blockEnd = Block.end();
       currentInstruction != blockEnd; ++currentInstruction) {

    MachineInstr &multiplicationInstruction = *currentInstruction;
    unsigned multiplicationOpcode = multiplicationInstruction.getOpcode();

    // Identify floating-point multiplication operations
    if (multiplicationOpcode != X86::MULSSrr &&
        multiplicationOpcode != X86::MULSDrr &&
        multiplicationOpcode != X86::VMULSSrr &&
        multiplicationOpcode != X86::VMULSDrr &&
        multiplicationOpcode != X86::VMULPSrr &&
        multiplicationOpcode != X86::VMULPDrr)
      continue;

    // Verify single use of multiplication result
    Register multiplicationResult =
        multiplicationInstruction.getOperand(0).getReg();
    if (!registerInformation.hasOneUse(multiplicationResult))
      continue;

    // Locate subsequent arithmetic operation (subtraction or addition)
    bool isOperandOrderReversed = false;
    auto arithmeticInstructionIterator = std::next(currentInstruction);

    for (; arithmeticInstructionIterator != blockEnd;
         ++arithmeticInstructionIterator) {
      MachineInstr &arithmeticInstruction = *arithmeticInstructionIterator;
      unsigned arithmeticOpcode = arithmeticInstruction.getOpcode();

      bool isSubtraction = arithmeticOpcode == X86::SUBSSrr ||
                           arithmeticOpcode == X86::SUBSDrr ||
                           arithmeticOpcode == X86::VSUBSSrr ||
                           arithmeticOpcode == X86::VSUBSDrr;

      bool isAddition = arithmeticOpcode == X86::VADDSSrr ||
                        arithmeticOpcode == X86::VADDSDrr;

      if (isSubtraction || isAddition) {
        // Check if multiplication result is used as operand
        if (arithmeticInstruction.getOperand(1).getReg() ==
                multiplicationResult ||
            arithmeticInstruction.getOperand(2).getReg() ==
                multiplicationResult)
          break;
      }
    }

    if (arithmeticInstructionIterator == blockEnd)
      continue;

    MachineInstr &arithmeticInstruction = *arithmeticInstructionIterator;

    // Validate operand usage pattern
    if (!validateOperandPattern(multiplicationInstruction,
                                arithmeticInstruction, isOperandOrderReversed))
      continue;

    // Determine appropriate fused multiply-add instruction
    auto fusedOpcode = determineFusedOpcode(multiplicationOpcode,
                                            arithmeticInstruction.getOpcode(),
                                            isOperandOrderReversed);
    if (!fusedOpcode)
      continue;

    // Select constant register index based on operand order
    unsigned constantOperandIndex = isOperandOrderReversed ? 2 : 1;
    Register constantRegister =
        arithmeticInstruction.getOperand(constantOperandIndex).getReg();

    // Construct fused instruction
    BuildMI(Block, arithmeticInstruction, arithmeticInstruction.getDebugLoc(),
            InstructionInfo->get(*fusedOpcode),
            arithmeticInstruction.getOperand(0).getReg())
        .addReg(multiplicationInstruction.getOperand(1).getReg(),
                getRegState(multiplicationInstruction.getOperand(1)))
        .addReg(multiplicationInstruction.getOperand(2).getReg(),
                getRegState(multiplicationInstruction.getOperand(2)))
        .addReg(constantRegister, getRegState(arithmeticInstruction.getOperand(
                                      constantOperandIndex)));

    instructionsToRemove.push_back(&multiplicationInstruction);
    instructionsToRemove.push_back(&arithmeticInstruction);
    blockModified = true;
  }

  // Remove original instructions in reverse order
  for (auto *instruction : reverse(instructionsToRemove))
    instruction->eraseFromParent();

  return blockModified;
}

std::optional<unsigned> FloatingPointFusionPass::determineFusedOpcode(
    unsigned multiplicationOpcode, unsigned arithmeticOpcode,
    bool isOperandOrderReversed) const {
  // Handle addition case: -(a*b) + c transformation
  if (arithmeticOpcode == X86::VADDSSrr)
    return isOperandOrderReversed ? X86::VFNMADD213SSr : X86::VFNMADD132SSr;
  if (arithmeticOpcode == X86::VADDSDrr)
    return isOperandOrderReversed ? X86::VFNMADD213SDr : X86::VFNMADD132SDr;

  // Handle subtraction case based on operand order
  bool multiplicationResultIsFirstOperand = isOperandOrderReversed;

  switch (multiplicationOpcode) {
  case X86::MULSSrr:
  case X86::VMULSSrr:
    return multiplicationResultIsFirstOperand ? X86::VFMSUB213SSr
                                              : X86::VFNMADD213SSr;

  case X86::MULSDrr:
  case X86::VMULSDrr:
    return multiplicationResultIsFirstOperand ? X86::VFMSUB213SDr
                                              : X86::VFNMADD213SDr;

  case X86::VMULPSrr:
    return multiplicationResultIsFirstOperand ? X86::VFMSUB213PSr
                                              : X86::VFNMADD213PSr;

  case X86::VMULPDrr:
    return multiplicationResultIsFirstOperand ? X86::VFMSUB213PDr
                                              : X86::VFNMADD213PDr;

  default:
    return std::nullopt;
  }
}

bool FloatingPointFusionPass::validateOperandPattern(
    const MachineInstr &multiplicationInstruction,
    const MachineInstr &arithmeticInstruction,
    bool &isOperandOrderReversed) const {

  Register multiplicationResult =
      multiplicationInstruction.getOperand(0).getReg();
  unsigned arithmeticOpcode = arithmeticInstruction.getOpcode();
  Register firstOperand = arithmeticInstruction.getOperand(1).getReg();
  Register secondOperand = arithmeticInstruction.getOperand(2).getReg();

  if (arithmeticOpcode == X86::VADDSSrr || arithmeticOpcode == X86::VADDSDrr) {
    // Addition patterns: -(tmp) + c or c + tmp
    if (firstOperand == multiplicationResult) {
      isOperandOrderReversed = true;
      return true;
    }
    if (secondOperand == multiplicationResult) {
      isOperandOrderReversed = false;
      return true;
    }
    return false;
  }

  // Subtraction patterns: tmp - c or c - tmp
  if (firstOperand == multiplicationResult) {
    isOperandOrderReversed = true;
    return true;
  }
  if (secondOperand == multiplicationResult) {
    isOperandOrderReversed = false;
    return true;
  }

  return false;
}

} // namespace

static RegisterPass<FloatingPointFusionPass>
    RegistrationPoint("floating-point-fusion",
                      "Fuse floating-point multiplication and "
                      "addition/subtraction into FMA instructions",
                      false, false);