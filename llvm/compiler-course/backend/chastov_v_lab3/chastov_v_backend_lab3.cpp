#include "X86.h"
#include "X86InstrInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

namespace {
class X86BitwiseCombine : public MachineFunctionPass {
public:
  static char ID;
  X86BitwiseCombine() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool Modified = false;
    const X86InstrInfo *TII =
        static_cast<const X86InstrInfo *>(MF.getSubtarget().getInstrInfo());

    for (MachineBasicBlock &Block : MF) {
      for (auto InstIter = Block.begin(); InstIter != Block.end();) {
        MachineInstr &CurrentInstr = *InstIter;
        unsigned BaseOp = CurrentInstr.getOpcode();
        unsigned EnhancedOp = convertToAVX(BaseOp);

        if (EnhancedOp) {
          BuildMI(Block, InstIter, CurrentInstr.getDebugLoc(),
                  TII->get(EnhancedOp), CurrentInstr.getOperand(0).getReg())
              .addReg(CurrentInstr.getOperand(1).getReg())
              .addReg(CurrentInstr.getOperand(2).getReg());

          InstIter = Block.erase(InstIter);
          Modified = true;
        } else {
          ++InstIter;
        }
      }
    }
    return Modified;
  }

private:
  unsigned convertToAVX(unsigned BaseOpcode) const {
    switch (BaseOpcode) {
    case X86::PANDrr:
      return X86::VPANDrr;
    case X86::PORrr:
      return X86::VPORrr;
    case X86::PXORrr:
      return X86::VPXORrr;
    case X86::ANDPSrr:
      return X86::VANDPSrr;
    case X86::ORPSrr:
      return X86::VORPSrr;
    case X86::XORPSrr:
      return X86::VXORPSrr;
    case X86::PANDNrr:
      return X86::VPANDNrr;
    case X86::ANDPDrr:
      return X86::VANDPDrr;
    case X86::ORPDrr:
      return X86::VORPDrr;
    case X86::XORPDrr:
      return X86::VXORPDrr;
    default:
      return 0;
    }
  }
};

char X86BitwiseCombine::ID = 0;
} // namespace

static llvm::RegisterPass<X86BitwiseCombine>
    X("x86-logic-opt", "X86 Logical Operations Optimization Pass", false,
      false);
