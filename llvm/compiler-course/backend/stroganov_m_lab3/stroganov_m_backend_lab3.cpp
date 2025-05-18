#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

namespace {
class X86LogicOptPass : public MachineFunctionPass {
public:
  static char ID;
  X86LogicOptPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    const DenseMap<unsigned, unsigned> AVXOpcodeMap = {
        {X86::PANDrr, X86::VPANDrr},   {X86::PORrr, X86::VPORrr},
        {X86::PXORrr, X86::VPXORrr},   {X86::ANDPSrr, X86::VANDPSrr},
        {X86::ORPSrr, X86::VORPSrr},   {X86::XORPSrr, X86::VXORPSrr},
        {X86::PANDNrr, X86::VPANDNrr}, {X86::ANDPDrr, X86::VANDPDrr},
        {X86::ORPDrr, X86::VORPDrr},   {X86::XORPDrr, X86::VXORPDrr},
    };

    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        auto AVXIter = AVXOpcodeMap.find(Opc);
        if (AVXIter != AVXOpcodeMap.end()) {
          BuildMI(MBB, MII, MI.getDebugLoc(), TII->get(AVXIter->second),
                  MI.getOperand(0).getReg())
              .addReg(MI.getOperand(1).getReg())
              .addReg(MI.getOperand(2).getReg());

          MII = MBB.erase(MII);
          Changed = true;
        } else {
          ++MII;
        }
      }
    }

    return Changed;
  }
};

char X86LogicOptPass::ID = 0;
} // namespace

static llvm::RegisterPass<X86LogicOptPass>
    X("x86-logic-opt", "X86 Logical Operations Optimization Pass", false,
      false);
