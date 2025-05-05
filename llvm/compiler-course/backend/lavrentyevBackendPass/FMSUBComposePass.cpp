#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Passes/PassBuilder.h"

#define DEBUG_TYPE "fmsub-compose"

using namespace llvm;

namespace {
class FMSUBComposePass : public MachineFunctionPass {
public:
  static char ID;
  FMSUBComposePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    if (!ST.hasFMA())
      return false;

    const X86InstrInfo *TII = ST.getInstrInfo();
    bool Changed = false;

    for (auto &MBB : MF) {
      SmallVector<MachineInstr *, 8> ToErase;

      for (auto MIit = MBB.instr_begin(), ME = MBB.instr_end(); MIit != ME;
           ++MIit) {
        MachineInstr &MulMI = *MIit;
        unsigned MulOpc = MulMI.getOpcode();
        unsigned SubOpc = 0, FMAOpc = 0;

        switch (MulOpc) {
        case X86::MULPSrm:
          SubOpc = X86::SUBPSrr;
          FMAOpc = X86::VFMSUB213PSm;
          break;
        case X86::MULPDrm:
          SubOpc = X86::SUBPDrr;
          FMAOpc = X86::VFMSUB213PDm;
          break;
        case X86::MULSSrm:
          SubOpc = X86::SUBSSrr;
          FMAOpc = X86::VFMSUB213SSm;
          break;
        case X86::MULSDrm:
          SubOpc = X86::SUBSDrr;
          FMAOpc = X86::VFMSUB213SDm;
          break;
        default:
          continue;
        }

        MachineInstr *SubMI = nullptr;
        Register TmpReg = MulMI.getOperand(0).getReg();
        auto ScanIt = std::next(MIit);
        for (; ScanIt != ME; ++ScanIt) {
          for (auto &Op : ScanIt->operands()) {
            if (Op.isReg() && Op.isDef() && Op.getReg() == TmpReg) {
              ScanIt = ME;
              break;
            }
          }
          if (ScanIt == ME)
            break;
          if (ScanIt->getOpcode() == SubOpc &&
              ScanIt->getOperand(2).getReg() == TmpReg) {
            SubMI = &*ScanIt;
            break;
          }
        }
        if (!SubMI)
          continue;

        Register Dst = SubMI->getOperand(0).getReg();
        Register C = SubMI->getOperand(1).getReg();
        DebugLoc DL = MulMI.getDebugLoc();

        auto MIB = BuildMI(MBB, MulMI, DL, TII->get(FMAOpc), Dst);

        Register A = MulMI.getOperand(2).getReg();
        MIB.addReg(A);
        for (auto *MO : MulMI.memoperands()) {
          auto *MMO_nc = const_cast<MachineMemOperand *>(MO);
          MIB.addMemOperand(MMO_nc);
        }
        MIB.addReg(C);

        ToErase.push_back(&MulMI);
        ToErase.push_back(SubMI);
        Changed = true;
      }

      for (auto *MI : ToErase)
        MI->eraseFromParent();
    }

    return Changed;
  }
};

char FMSUBComposePass::ID = 0;

static RegisterPass<FMSUBComposePass>
    X("fmsub-compose-x86",
      "Compose MULrm+SUBrr into a single fused multiply‑subtract (mem‑reg)",
      false, false);

} // namespace
