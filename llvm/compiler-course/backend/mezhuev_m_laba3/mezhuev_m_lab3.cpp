#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {

class ArithmeticFusionOptimizer : public MachineFunctionPass {
public:
  static char ID;
  ArithmeticFusionOptimizer() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "X86 Arithmetic Operations Fusion";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const X86InstrInfo *InstrInfo = nullptr;

  bool processMachineBlock(MachineBasicBlock &MBB);
  unsigned selectFusedOperation(unsigned multiplicationOp,
                                unsigned arithmeticOp,
                                bool mulIsArithOp1) const;
  bool validateOperandPattern(const MachineInstr &mulInstr,
                              const MachineInstr &arithInstr,
                              bool &mulIsArithOp1) const;
};

char ArithmeticFusionOptimizer::ID = 0;

bool ArithmeticFusionOptimizer::runOnMachineFunction(MachineFunction &MF) {
  const auto &ST = MF.getSubtarget<X86Subtarget>();
  if (!ST.hasFMA()) {
    return false;
  }
  InstrInfo = ST.getInstrInfo();

  bool changed = false;
  for (auto &MBB : MF) {
    changed |= processMachineBlock(MBB);
  }
  return changed;
}

bool ArithmeticFusionOptimizer::processMachineBlock(MachineBasicBlock &MBB) {
  SmallVector<MachineInstr *, 8> instructionsForRemoval;
  auto &regInfo = MBB.getParent()->getRegInfo();
  bool modified = false;

  for (auto instIter = MBB.begin(), endIter = MBB.end(); instIter != endIter;
       ++instIter) {

    MachineInstr &currentInstr = *instIter;
    unsigned currentOpcode = currentInstr.getOpcode();

    if (currentOpcode != X86::MULSSrr && currentOpcode != X86::MULSDrr &&
        currentOpcode != X86::VMULSSrr && currentOpcode != X86::VMULSDrr &&
        currentOpcode != X86::VMULPSrr && currentOpcode != X86::VMULPDrr) {
      continue;
    }

    Register multiplicationDest = currentInstr.getOperand(0).getReg();
    if (!regInfo.hasOneUse(multiplicationDest)) {
      continue;
    }

    auto arithmeticIter = std::next(instIter);
    for (; arithmeticIter != endIter; ++arithmeticIter) {
      MachineInstr &arithmeticInstr = *arithmeticIter;
      unsigned arithmeticOp = arithmeticInstr.getOpcode();

      bool isSubtractionOp =
          (arithmeticOp == X86::SUBSSrr || arithmeticOp == X86::SUBSDrr ||
           arithmeticOp == X86::VSUBSSrr || arithmeticOp == X86::VSUBSDrr);

      if (isSubtractionOp) {
        if (arithmeticInstr.getOperand(1).getReg() == multiplicationDest ||
            arithmeticInstr.getOperand(2).getReg() == multiplicationDest) {
          break;
        }
      }
    }

    if (arithmeticIter == endIter)
      continue;

    MachineInstr &arithmeticInstr = *arithmeticIter;

    bool mulIsArithOp1 = false;
    if (!validateOperandPattern(currentInstr, arithmeticInstr, mulIsArithOp1)) {
      continue;
    }

    unsigned fusedOpcode = selectFusedOperation(
        currentOpcode, arithmeticInstr.getOpcode(), mulIsArithOp1);

    if (fusedOpcode == 0)
      continue;

    unsigned otherOperandIdx = mulIsArithOp1 ? 2 : 1;
    Register otherOperandReg =
        arithmeticInstr.getOperand(otherOperandIdx).getReg();

    MachineInstrBuilder fusedInstr = BuildMI(
        MBB, arithmeticInstr, arithmeticInstr.getDebugLoc(),
        InstrInfo->get(fusedOpcode), arithmeticInstr.getOperand(0).getReg());

    fusedInstr
        .addReg(currentInstr.getOperand(1).getReg(),
                getRegState(currentInstr.getOperand(1)))
        .addReg(currentInstr.getOperand(2).getReg(),
                getRegState(currentInstr.getOperand(2)));

    fusedInstr.addReg(otherOperandReg,
                      getRegState(arithmeticInstr.getOperand(otherOperandIdx)));

    instructionsForRemoval.push_back(&currentInstr);
    instructionsForRemoval.push_back(&arithmeticInstr);
    modified = true;
  }

  for (auto *instr : llvm::reverse(instructionsForRemoval)) {
    instr->eraseFromParent();
  }

  return modified;
}

unsigned
ArithmeticFusionOptimizer::selectFusedOperation(unsigned multiplicationOp,
                                                unsigned arithmeticOp,
                                                bool mulIsArithOp1) const {

  auto isSub = [&](unsigned op) {
    return (op == X86::SUBSSrr || op == X86::SUBSDrr || op == X86::VSUBSSrr ||
            op == X86::VSUBSDrr);
  };

  if (!isSub(arithmeticOp))
    return 0;

  switch (multiplicationOp) {
  case X86::MULSSrr:
  case X86::VMULSSrr:
    return mulIsArithOp1 ? X86::VFMSUB213SSr : X86::VFNMADD213SSr;

  case X86::MULSDrr:
  case X86::VMULSDrr:
    return mulIsArithOp1 ? X86::VFMSUB213SDr : X86::VFNMADD213SDr;

  case X86::VMULPSrr:
    return mulIsArithOp1 ? X86::VFMSUB213PSr : X86::VFNMADD213PSr;

  case X86::VMULPDrr:
    return mulIsArithOp1 ? X86::VFMSUB213PDr : X86::VFNMADD213PDr;

  default:
    return 0;
  }
}

bool ArithmeticFusionOptimizer::validateOperandPattern(
    const MachineInstr &mulInstr, const MachineInstr &arithInstr,
    bool &mulIsArithOp1) const {

  Register mulResult = mulInstr.getOperand(0).getReg();
  Register arithOp1 = arithInstr.getOperand(1).getReg();
  Register arithOp2 = arithInstr.getOperand(2).getReg();

  if (arithOp1 == mulResult) {
    mulIsArithOp1 = true;
    return true;
  }

  if (arithOp2 == mulResult) {
    mulIsArithOp1 = false;
    return true;
  }

  return false;
}

} // namespace

static RegisterPass<ArithmeticFusionOptimizer>
    Z("x86-arith-fusion", "X86 Arithmetic Operations Fusion Pass", false,
      false);
