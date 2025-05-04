#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "fma-decompose"

using namespace llvm;

namespace {

class FMADecomposePass : public MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (auto &MBB : MF) {
      Changed |= processBasicBlock(MBB, TII, MRI);
    }

    return Changed;
  }

private:
  bool processBasicBlock(MachineBasicBlock &MBB, const X86InstrInfo *TII,
                         MachineRegisterInfo &MRI) {
    bool Changed = false;
    SmallVector<MachineInstr *, 8> ToErase;

    for (auto &MI : MBB) {
      if (!processInstruction(MI, MBB, TII, MRI, ToErase)) {
        continue;
      }
      Changed = true;
    }

    for (auto *MI : ToErase) {
      MI->eraseFromParent();
    }

    return Changed;
  }

  bool processInstruction(MachineInstr &MI, MachineBasicBlock &MBB,
                          const X86InstrInfo *TII, MachineRegisterInfo &MRI,
                          SmallVectorImpl<MachineInstr *> &ToErase) {
    unsigned MulOpc, AddOpc;
    if (!getFMADecompositionInfo(MI.getOpcode(), MulOpc, AddOpc)) {
      return false;
    }

    decomposeFMAInstruction(MI, MBB, TII, MRI, MulOpc, AddOpc);
    ToErase.push_back(&MI);
    return true;
  }

  bool getFMADecompositionInfo(unsigned Opc, unsigned &MulOpc,
                               unsigned &AddOpc) const {
    switch (Opc) {
    case X86::VFMADD132PSr:
    case X86::VFMADD213PSr:
    case X86::VFMADD231PSr:
      MulOpc = X86::MULPSrr;
      AddOpc = X86::ADDPSrr;
      return true;
    case X86::VFMADD132PDr:
    case X86::VFMADD213PDr:
    case X86::VFMADD231PDr:
      MulOpc = X86::MULPDrr;
      AddOpc = X86::ADDPDrr;
      return true;
    case X86::VFMADD132SSr:
    case X86::VFMADD213SSr:
    case X86::VFMADD231SSr:
      MulOpc = X86::MULSSrr;
      AddOpc = X86::ADDSSrr;
      return true;
    case X86::VFMADD132SDr:
    case X86::VFMADD213SDr:
    case X86::VFMADD231SDr:
      MulOpc = X86::MULSDrr;
      AddOpc = X86::ADDSDrr;
      return true;
    default:
      return false;
    }
  }

  void decomposeFMAInstruction(MachineInstr &MI, MachineBasicBlock &MBB,
                               const X86InstrInfo *TII,
                               MachineRegisterInfo &MRI, unsigned MulOpc,
                               unsigned AddOpc) {
    Register Dst = MI.getOperand(0).getReg();
    Register A = MI.getOperand(1).getReg();
    Register B = MI.getOperand(2).getReg();
    Register C = MI.getOperand(3).getReg();
    DebugLoc DL = MI.getDebugLoc();

    // Создаем временный регистр того же класса, что и операнды
    const TargetRegisterClass *RC = MRI.getRegClass(A);
    Register Tmp = MRI.createVirtualRegister(RC);

    // Вставляем умножение перед оригинальной инструкцией
    BuildMI(MBB, MI, DL, TII->get(MulOpc), Tmp).addReg(A).addReg(B);

    // Вставляем сложение перед оригинальной инструкцией
    BuildMI(MBB, MI, DL, TII->get(AddOpc), Dst).addReg(C).addReg(Tmp);
  }

  StringRef getPassName() const override { return "FMA Decomposition Pass"; }
};

char FMADecomposePass::ID = 0;

} // namespace

static RegisterPass<FMADecomposePass>
    X("fma-decompose", "Decompose FMA instructions into MUL+ADD", false, false);
