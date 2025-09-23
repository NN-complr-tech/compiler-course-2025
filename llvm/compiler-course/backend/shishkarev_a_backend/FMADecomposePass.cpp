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

class FMADecomposePass : public llvm::MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &ST = MF.getSubtarget<llvm::X86Subtarget>();
    // skip if fma not supported
    if (!ST.hasFMA())
      return false;
    const llvm::X86InstrInfo *TII = ST.getInstrInfo();
    llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> ToErase;
      // first pass: insert mul+add and mark original fma for erasure
      for (auto &MI : MBB) {
        unsigned Opc = MI.getOpcode();
        unsigned MulOpc = 0, AddOpc = 0;

        // skip non-fma instructions
        switch (Opc) {
        // packed single-precision fma
        case llvm::X86::VFMADD132PSr:
        case llvm::X86::VFMADD213PSr:
        case llvm::X86::VFMADD231PSr:
          MulOpc = llvm::X86::MULPSrr;
          AddOpc = llvm::X86::ADDPSrr;
          break;
        // packed double-precision fma
        case llvm::X86::VFMADD132PDr:
        case llvm::X86::VFMADD213PDr:
        case llvm::X86::VFMADD231PDr:
          MulOpc = llvm::X86::MULPDrr;
          AddOpc = llvm::X86::ADDPDrr;
          break;
        // scalar single-precision fma
        case llvm::X86::VFMADD132SSr:
        case llvm::X86::VFMADD213SSr:
        case llvm::X86::VFMADD231SSr:
          MulOpc = llvm::X86::MULSSrr;
          AddOpc = llvm::X86::ADDSSrr;
          break;
        // scalar double-precision fma
        case llvm::X86::VFMADD132SDr:
        case llvm::X86::VFMADD213SDr:
        case llvm::X86::VFMADD231SDr:
          MulOpc = llvm::X86::MULSDrr;
          AddOpc = llvm::X86::ADDSDrr;
          break;
        default:
          continue;
        }

        // operands: dst, src1, src2, src3
        llvm::DebugLoc DL = MI.getDebugLoc();
        llvm::Register Dst = MI.getOperand(0).getReg();
        llvm::Register A = MI.getOperand(1).getReg();
        llvm::Register B = MI.getOperand(2).getReg();
        llvm::Register C = MI.getOperand(3).getReg();

        // create a tmp vreg of the same class as A
        const llvm::TargetRegisterClass *RC = MRI.getRegClass(A);
        llvm::Register Tmp = MRI.createVirtualRegister(RC);

        // insert mul before this fma
        llvm::BuildMI(MBB, MI, DL, TII->get(MulOpc), Tmp).addReg(A).addReg(B);

        // insert add before this fma
        llvm::BuildMI(MBB, MI, DL, TII->get(AddOpc), Dst).addReg(C).addReg(Tmp);

        // mark original fma for erasure
        ToErase.push_back(&MI);
        Changed = true;
      }

      // second pass: actually remove all marked FMAs
      for (auto *MI : ToErase)
        MI->eraseFromParent();
    }

    return Changed;
  }
};

char FMADecomposePass::ID = 0;

} // end anonymous namespace

static llvm::RegisterPass<FMADecomposePass>
    X("fma-decompose-x86",
      "Decomposes single FMA instructions into MUL and ADD ones", false, false);