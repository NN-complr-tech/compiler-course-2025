#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "fma-decompose"

namespace {
class FMAFlatten : public MachineFunctionPass {
public:
  static char ID;
  FMAFlatten() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    auto &ST = MF.getSubtarget<X86Subtarget>();
    auto *TII = ST.getInstrInfo();
    auto &MRI = MF.getRegInfo();
    bool Changed = false;

    for (auto &Block : MF) {
      for (auto Inst = Block.begin(), End = Block.end(); Inst != End;) {
        MachineInstr &MI = *Inst++;
        unsigned MulOp, AddOp;
        unsigned A_idx, B_idx, C_idx;

        if (!identifyFMA(MI.getOpcode(), MulOp, AddOp, A_idx, B_idx, C_idx))
          continue;

        Register Dst = MI.getOperand(0).getReg();
        Register A = MI.getOperand(A_idx).getReg();
        Register B = MI.getOperand(B_idx).getReg();
        Register C = MI.getOperand(C_idx).getReg();
        DebugLoc Loc = MI.getDebugLoc();

        const TargetRegisterClass *RegClass = MRI.getRegClass(A);
        Register Temp = MRI.createVirtualRegister(RegClass);

        BuildMI(Block, MI, Loc, TII->get(MulOp), Temp).addReg(A).addReg(B);
        BuildMI(Block, MI, Loc, TII->get(AddOp), Dst).addReg(C).addReg(Temp);
        MI.eraseFromParent();
        Changed = true;
      }
    }

    return Changed;
  }

private:
  bool identifyFMA(unsigned Op, unsigned &Mul, unsigned &Add, unsigned &A_idx,
                   unsigned &B_idx, unsigned &C_idx) {
    switch (Op) {
    // Packed single-precision
    case X86::VFMADD132PSr:
      Mul = X86::MULPSrr;
      Add = X86::ADDPSrr;
      A_idx = 1;
      B_idx = 3;
      C_idx = 2;
      return true;
    case X86::VFMADD213PSr:
      Mul = X86::MULPSrr;
      Add = X86::ADDPSrr;
      A_idx = 2;
      B_idx = 1;
      C_idx = 3;
      return true;
    case X86::VFMADD231PSr:
      Mul = X86::MULPSrr;
      Add = X86::ADDPSrr;
      A_idx = 2;
      B_idx = 3;
      C_idx = 1;
      return true;

    // Packed double-precision
    case X86::VFMADD132PDr:
      Mul = X86::MULPDrr;
      Add = X86::ADDPDrr;
      A_idx = 1;
      B_idx = 3;
      C_idx = 2;
      return true;
    case X86::VFMADD213PDr:
      Mul = X86::MULPDrr;
      Add = X86::ADDPDrr;
      A_idx = 2;
      B_idx = 1;
      C_idx = 3;
      return true;
    case X86::VFMADD231PDr:
      Mul = X86::MULPDrr;
      Add = X86::ADDPDrr;
      A_idx = 2;
      B_idx = 3;
      C_idx = 1;
      return true;

    // Scalar single-precision
    case X86::VFMADD132SSr:
      Mul = X86::MULSSrr;
      Add = X86::ADDSSrr;
      A_idx = 1;
      B_idx = 3;
      C_idx = 2;
      return true;
    case X86::VFMADD213SSr:
      Mul = X86::MULSSrr;
      Add = X86::ADDSSrr;
      A_idx = 2;
      B_idx = 1;
      C_idx = 3;
      return true;
    case X86::VFMADD231SSr:
      Mul = X86::MULSSrr;
      Add = X86::ADDSSrr;
      A_idx = 2;
      B_idx = 3;
      C_idx = 1;
      return true;

    // Scalar double-precision
    case X86::VFMADD132SDr:
      Mul = X86::MULSDrr;
      Add = X86::ADDSDrr;
      A_idx = 1;
      B_idx = 3;
      C_idx = 2;
      return true;
    case X86::VFMADD213SDr:
      Mul = X86::MULSDrr;
      Add = X86::ADDSDrr;
      A_idx = 2;
      B_idx = 1;
      C_idx = 3;
      return true;
    case X86::VFMADD231SDr:
      Mul = X86::MULSDrr;
      Add = X86::ADDSDrr;
      A_idx = 2;
      B_idx = 3;
      C_idx = 1;
      return true;

    default:
      return false;
    }
  }
};

char FMAFlatten::ID = 0;
} // namespace

static llvm::RegisterPass<FMAFlatten>
    X("FMA-flatten-x86", "Decomposes FMA instructions into MUL and ADD", false,
      false);
