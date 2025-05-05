#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"

#define DEBUG_TYPE "fma-decompose-x86"

using namespace llvm;

namespace {

static bool getDecomposedOpcodes(unsigned Opcode, unsigned &MulOpc,
                                 unsigned &AddOpc) {
  switch (Opcode) {
  case X86::VFMADD132SSr:
  case X86::VFMADD213SSr:
  case X86::VFMADD231SSr:
    MulOpc = X86::VMULSSrr;
    AddOpc = X86::VADDSSrr;
    return true;
  case X86::VFMADD132PSr:
  case X86::VFMADD213PSr:
  case X86::VFMADD231PSr:
    MulOpc = X86::VMULPSrr;
    AddOpc = X86::VADDPSrr;
    return true;
  case X86::VFMADD132SDr:
  case X86::VFMADD213SDr:
  case X86::VFMADD231SDr:
    MulOpc = X86::VMULSDrr;
    AddOpc = X86::VADDSDrr;
    return true;
  case X86::VFMADD132PDr:
  case X86::VFMADD213PDr:
  case X86::VFMADD231PDr:
    MulOpc = X86::VMULPDrr;
    AddOpc = X86::VADDPDrr;
    return true;
  default:
    return false;
  }
}

class FmaDecompose : public MachineFunctionPass {
public:
  static char ID;
  FmaDecompose() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86InstrInfo *TII =
        static_cast<const X86InstrInfo *>(MF.getSubtarget().getInstrInfo());
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (auto &MBB : MF) {
      SmallVector<MachineInstr *, 4> ToErase;

      for (auto &MI : MBB) {
        unsigned Opcode = MI.getOpcode();
        DebugLoc DL = MI.getDebugLoc();

        unsigned MulOpc = 0, AddOpc = 0;
        if (!getDecomposedOpcodes(Opcode, MulOpc, AddOpc))
          continue;

        if (MI.getNumExplicitOperands() < 4)
          continue;
        if (!llvm::all_of(MI.operands(), [](const llvm::MachineOperand &Op) {
              return Op.isReg();
            })) {
          continue;
        }

        Register Dest = MI.getOperand(0).getReg();
        Register a = MI.getOperand(1).getReg();
        Register b = MI.getOperand(2).getReg();
        Register c = MI.getOperand(3).getReg();

        const TargetRegisterClass* RC = MRI.getRegClass(a);
        Register MulTmp = MRI.createVirtualRegister(RC);

        switch (Opcode) {
        case X86::VFMADD132SSr:
        case X86::VFMADD132SDr:
        case X86::VFMADD132PSr:
        case X86::VFMADD132PDr:

            BuildMI(MBB, MI, DL, TII->get(MulOpc), MulTmp).addReg(a).addReg(c);
            BuildMI(MBB, MI, DL, TII->get(AddOpc), Dest).addReg(b).addReg(MulTmp);
            break;
        case X86::VFMADD213SSr:
        case X86::VFMADD213SDr:
        case X86::VFMADD213PSr:
        case X86::VFMADD213PDr:

            BuildMI(MBB, MI, DL, TII->get(MulOpc), MulTmp).addReg(a).addReg(b);
            BuildMI(MBB, MI, DL, TII->get(AddOpc), Dest).addReg(c).addReg(MulTmp);
            break;
        case X86::VFMADD231SSr:
        case X86::VFMADD231SDr:
        case X86::VFMADD231PSr:
        case X86::VFMADD231PDr:

            BuildMI(MBB, MI, DL, TII->get(MulOpc), MulTmp).addReg(c).addReg(b);
            BuildMI(MBB, MI, DL, TII->get(AddOpc), Dest).addReg(a).addReg(MulTmp);
            break;
        default:
            continue;
        }

        ToErase.push_back(&MI);
        Changed = true;
      }

      for (MachineInstr *MI : ToErase)
        MI->eraseFromParent();
    }

    return Changed;
  }
};

} // namespace

char FmaDecompose::ID = 0;
static RegisterPass<FmaDecompose>
    X("decompose-fma", "Decompose FMA into MUL + ADD (SS/PS/SD/PD)", false,
      false);
