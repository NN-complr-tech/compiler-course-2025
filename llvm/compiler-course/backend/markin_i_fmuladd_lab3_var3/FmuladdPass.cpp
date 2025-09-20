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

#define DEBUG_TYPE "fma-decomposition"

namespace {

struct FusedMultiplyAddInfo {
  unsigned Instructions[3];
  unsigned MultiplyOp;
  unsigned AdditionOp;
};

static const FusedMultiplyAddInfo FMAInstructionSet[] = {
    {{llvm::X86::VFMADD132PSr, llvm::X86::VFMADD213PSr,
      llvm::X86::VFMADD231PSr},
     llvm::X86::MULPSrr,
     llvm::X86::ADDPSrr},

    {{llvm::X86::VFMADD132PDr, llvm::X86::VFMADD213PDr,
      llvm::X86::VFMADD231PDr},
     llvm::X86::MULPDrr,
     llvm::X86::ADDPDrr},

    {{llvm::X86::VFMADD132SSr, llvm::X86::VFMADD213SSr,
      llvm::X86::VFMADD231SSr},
     llvm::X86::MULSSrr,
     llvm::X86::ADDSSrr},

    {{llvm::X86::VFMADD132SDr, llvm::X86::VFMADD213SDr,
      llvm::X86::VFMADD231SDr},
     llvm::X86::MULSDrr,
     llvm::X86::ADDSDrr},
};

bool identifyFMA(unsigned Opcode, unsigned &MultiplyOp, unsigned &AdditionOp,
                 unsigned &FMAVariant) {
  for (const auto &Group : FMAInstructionSet) {
    for (unsigned i = 0; i < 3; ++i) {
      if (Group.Instructions[i] == Opcode) {
        MultiplyOp = Group.MultiplyOp;
        AdditionOp = Group.AdditionOp;
        FMAVariant = i;
        return true;
      }
    }
  }
  return false;
}

void transformFMA(llvm::MachineInstr &CurrentMI,
                  llvm::MachineBasicBlock &CurrentMBB,
                  const llvm::X86InstrInfo *TargetInfo,
                  llvm::MachineRegisterInfo &RegInfo, unsigned MultiplyOp,
                  unsigned AdditionOp, unsigned FMAVariant) {

  const llvm::DebugLoc &DebugLocation = CurrentMI.getDebugLoc();

  llvm::Register Destination = CurrentMI.getOperand(0).getReg();
  llvm::Register Operand1 = CurrentMI.getOperand(1).getReg();
  llvm::Register Operand2 = CurrentMI.getOperand(2).getReg();
  llvm::Register Operand3 = CurrentMI.getOperand(3).getReg();

  llvm::Register MultiplyLHS, MultiplyRHS, AdditionSource;

  switch (FMAVariant) {
  case 0:
    MultiplyLHS = Operand1;
    MultiplyRHS = Operand3;
    AdditionSource = Operand2;
    break;
  case 1:
    MultiplyLHS = Operand1;
    MultiplyRHS = Operand2;
    AdditionSource = Operand3;
    break;
  case 2:
    MultiplyLHS = Operand2;
    MultiplyRHS = Operand3;
    AdditionSource = Operand1;
    break;
  default:
    llvm_unreachable("Недопустимый индекс FMA");
  }

  const llvm::TargetRegisterClass *RegisterClasse =
      RegInfo.getRegClass(MultiplyLHS);
  llvm::Register TemporaryRegister =
      RegInfo.createVirtualRegister(RegisterClasse);

  llvm::BuildMI(CurrentMBB, CurrentMI, DebugLocation,
                TargetInfo->get(MultiplyOp), TemporaryRegister)
      .addReg(MultiplyLHS)
      .addReg(MultiplyRHS);

  llvm::BuildMI(CurrentMBB, CurrentMI, DebugLocation,
                TargetInfo->get(AdditionOp), Destination)
      .addReg(AdditionSource)
      .addReg(TemporaryRegister);
}

class FMADecomposePass : public llvm::MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &Subtarget = MF.getSubtarget<llvm::X86Subtarget>();
    if (!Subtarget.hasFMA())
      return false;

    const llvm::X86InstrInfo *TargetInfo = Subtarget.getInstrInfo();
    llvm::MachineRegisterInfo &RegInfo = MF.getRegInfo();
    bool Changed = false;

    for (llvm::MachineBasicBlock &BasicBlock : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> InstructionsToRemove;

      for (llvm::MachineInstr &Instruction :
           llvm::make_early_inc_range(BasicBlock)) {
        unsigned MultiplyOp, AdditionOp, FMAVariant;
        if (!identifyFMA(Instruction.getOpcode(), MultiplyOp, AdditionOp,
                         FMAVariant))
          continue;

        transformFMA(Instruction, BasicBlock, TargetInfo, RegInfo, MultiplyOp,
                     AdditionOp, FMAVariant);
        InstructionsToRemove.push_back(&Instruction);
        Changed = true;
      }

      for (llvm::MachineInstr *Instruction : InstructionsToRemove) {
        Instruction->eraseFromParent();
      }
    }

    return Changed;
  }
};

char FMADecomposePass::ID = 0;

} // namespace

static llvm::RegisterPass<FMADecomposePass>
    X("fma-decompose-x86", "Разложение FMA инструкций на MUL и ADD", false,
      false);
