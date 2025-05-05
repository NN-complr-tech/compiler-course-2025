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

#define DEBUG_TYPE "fma-decompose"

namespace {

struct FMAOpcodeGroup {
  unsigned FMAOps[3];
  unsigned MulOp;
  unsigned AddOp;
};

static const FMAOpcodeGroup FMAOpcodes[] = {
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

bool matchFMA(unsigned Opc, unsigned &MulOp, unsigned &AddOp) {
  for (const auto &Group : FMAOpcodes) {
    for (unsigned FMA : Group.FMAOps) {
      if (FMA == Opc) {
        MulOp = Group.MulOp;
        AddOp = Group.AddOp;
        return true;
      }
    }
  }
  return false;
}

void decomposeFMA(llvm::MachineInstr &MI, llvm::MachineBasicBlock &MBB,
                  const llvm::X86InstrInfo *TII, llvm::MachineRegisterInfo &MRI,
                  unsigned MulOp, unsigned AddOp) {
  const llvm::DebugLoc &DL = MI.getDebugLoc();

  llvm::Register Dst = MI.getOperand(0).getReg();
  llvm::Register A = MI.getOperand(1).getReg();
  llvm::Register B = MI.getOperand(2).getReg();
  llvm::Register C = MI.getOperand(3).getReg();

  const llvm::TargetRegisterClass *RC = MRI.getRegClass(A);
  llvm::Register Tmp = MRI.createVirtualRegister(RC);

  llvm::BuildMI(MBB, MI, DL, TII->get(MulOp), Tmp).addReg(A).addReg(B);
  llvm::BuildMI(MBB, MI, DL, TII->get(AddOp), Dst).addReg(C).addReg(Tmp);
}

class FMADecomposePass : public llvm::MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &ST = MF.getSubtarget<llvm::X86Subtarget>();
    if (!ST.hasFMA())
      return false;

    const llvm::X86InstrInfo *TII = ST.getInstrInfo();
    llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (llvm::MachineBasicBlock &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> ToErase;

      for (llvm::MachineInstr &MI : MBB) {
        unsigned MulOp, AddOp;
        if (!matchFMA(MI.getOpcode(), MulOp, AddOp))
          continue;

        decomposeFMA(MI, MBB, TII, MRI, MulOp, AddOp);
        ToErase.push_back(&MI);
        Changed = true;
      }

      for (llvm::MachineInstr *MI : ToErase) {
        MI->eraseFromParent();
      }
    }

    return Changed;
  }
};

char FMADecomposePass::ID = 0;

} // namespace

static llvm::RegisterPass<FMADecomposePass>
    X("fma-decompose-x86",
      "Decomposes single FMA instructions into MUL and ADD ones", false, false);
