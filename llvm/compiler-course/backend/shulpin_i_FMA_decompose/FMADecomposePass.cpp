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
  unsigned FMAOps[3]; // 132, 213, 231
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

bool matchFMA(unsigned Opc, unsigned &MulOp, unsigned &AddOp,
              unsigned &FMAIndex) {
  for (const auto &Group : FMAOpcodes) {
    for (unsigned i = 0; i < 3; ++i) {
      if (Group.FMAOps[i] == Opc) {
        MulOp = Group.MulOp;
        AddOp = Group.AddOp;
        FMAIndex = i; // 0 = 132, 1 = 213, 2 = 231
        return true;
      }
    }
  }
  return false;
}

void decomposeFMA(llvm::MachineInstr &MI, llvm::MachineBasicBlock &MBB,
                  const llvm::X86InstrInfo *TII, llvm::MachineRegisterInfo &MRI,
                  unsigned MulOp, unsigned AddOp, unsigned FMAIndex) {

  const llvm::DebugLoc &DL = MI.getDebugLoc();

  llvm::Register Dst = MI.getOperand(0).getReg();
  llvm::Register Op1 = MI.getOperand(1).getReg();
  llvm::Register Op2 = MI.getOperand(2).getReg();
  llvm::Register Op3 = MI.getOperand(3).getReg();

  llvm::Register MulLHS, MulRHS, AddSrc;

  // FMA semantics:
  // 132: (Op1 * Op3) + Op2
  // 213: (Op1 * Op2) + Op3
  // 231: (Op2 * Op3) + Op1

  switch (FMAIndex) {
  case 0: // 132
    MulLHS = Op1;
    MulRHS = Op3;
    AddSrc = Op2;
    break;
  case 1: // 213
    MulLHS = Op1;
    MulRHS = Op2;
    AddSrc = Op3;
    break;
  case 2: // 231
    MulLHS = Op2;
    MulRHS = Op3;
    AddSrc = Op1;
    break;
  default:
    llvm_unreachable("Invalid FMA index");
  }

  const llvm::TargetRegisterClass *RC = MRI.getRegClass(MulLHS);
  llvm::Register Tmp = MRI.createVirtualRegister(RC);

  llvm::BuildMI(MBB, MI, DL, TII->get(MulOp), Tmp)
      .addReg(MulLHS)
      .addReg(MulRHS);

  llvm::BuildMI(MBB, MI, DL, TII->get(AddOp), Dst).addReg(AddSrc).addReg(Tmp);
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

      for (llvm::MachineInstr &MI : llvm::make_early_inc_range(MBB)) {
        unsigned MulOp, AddOp, FMAIndex;
        if (!matchFMA(MI.getOpcode(), MulOp, AddOp, FMAIndex))
          continue;

        decomposeFMA(MI, MBB, TII, MRI, MulOp, AddOp, FMAIndex);
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