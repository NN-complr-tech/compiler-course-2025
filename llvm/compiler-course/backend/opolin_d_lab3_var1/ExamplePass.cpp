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
class AVXLogicCombinerPass : public MachineFunctionPass {
  const X86InstrInfo *TII = nullptr;
  MachineRegisterInfo *RegInfo = nullptr;
  static const DenseMap<unsigned, unsigned> scalarToAVX;

  bool tryFoldPair(MachineBasicBlock &MBB, MachineBasicBlock::iterator &it) {
    MachineInstr &curr = *it;
    if (curr.getNumOperands() < 3 || !curr.getOperand(1).isReg())
      return false;

    Register src = curr.getOperand(1).getReg();
    if (!RegInfo->hasOneUse(src))
      return false;

    MachineInstr *def = RegInfo->getUniqueVRegDef(src);
    if (!def)
      return false;
    unsigned opc1 = def->getOpcode();
    unsigned opc2 = curr.getOpcode();
    if (!scalarToAVX.count(opc1) || !scalarToAVX.count(opc2))
      return false;

    fuseInstructions(MBB, it, def, opc1, curr, opc2);
    def->eraseFromParent();
    it = MBB.erase(it);
    return true;
  }

  void fuseInstructions(MachineBasicBlock &MBB, MachineBasicBlock::iterator &it,
                        MachineInstr *defMI, unsigned opc1, MachineInstr &useMI,
                        unsigned opc2) {
    Register def1 = defMI->getOperand(1).getReg();
    Register def2 = defMI->getOperand(2).getReg();
    Register useDest = useMI.getOperand(0).getReg();
    Register useSrc = useMI.getOperand(2).getReg();

    bool isOR =
        (opc1 == X86::PORrr || opc1 == X86::ORPSrr || opc1 == X86::ORPDrr);
    bool isAND =
        (opc2 == X86::PANDrr || opc2 == X86::ANDPSrr || opc2 == X86::ANDPDrr);
    bool match = (useSrc == def1) || (useSrc == def2);
    if (isOR && isAND && match) {
      Register toCopy = (useSrc == def1) ? def1 : def2;
      BuildMI(MBB, it, useMI.getDebugLoc(), TII->get(X86::MOVAPSrr), useDest)
          .addReg(toCopy);
      return;
    }
    Register tmp = RegInfo->createVirtualRegister(
        RegInfo->getRegClass(useMI.getOperand(1).getReg()));
    DebugLoc dlDef = defMI->getDebugLoc();
    DebugLoc dlUse = useMI.getDebugLoc();

    BuildMI(MBB, it, dlDef, TII->get(scalarToAVX.at(opc1)), tmp)
        .addReg(defMI->getOperand(1).getReg())
        .addReg(defMI->getOperand(2).getReg());

    BuildMI(MBB, it, dlUse, TII->get(scalarToAVX.at(opc2)),
            useMI.getOperand(0).getReg())
        .addReg(tmp)
        .addReg(useMI.getOperand(2).getReg());
  }

  bool tryUpgradeSingle(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator &it) {
    MachineInstr &mi = *it;
    unsigned opc = mi.getOpcode();
    auto itMap = scalarToAVX.find(opc);
    if (itMap == scalarToAVX.end() || mi.getNumOperands() < 3)
      return false;

    DebugLoc dl = mi.getDebugLoc();
    BuildMI(MBB, it, dl, TII->get(itMap->second), mi.getOperand(0).getReg())
        .addReg(mi.getOperand(1).getReg())
        .addReg(mi.getOperand(2).getReg());
    it = MBB.erase(it);
    return true;
  }

public:
  static char ID;
  AVXLogicCombinerPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    TII = ST.getInstrInfo();
    RegInfo = &MF.getRegInfo();
    bool Changed = false;
    for (auto &MBB : MF) {
      for (auto it = MBB.begin(), end = MBB.end(); it != end;) {
        if (tryFoldPair(MBB, it)) {
          Changed = true;
          continue;
        }
        if (tryUpgradeSingle(MBB, it)) {
          Changed = true;
          continue;
        }
        ++it;
      }
    }
    return Changed;
  }
};

const DenseMap<unsigned, unsigned> AVXLogicCombinerPass::scalarToAVX = {
    {X86::PANDrr, X86::VPANDrr},   {X86::PORrr, X86::VPORrr},
    {X86::PXORrr, X86::VPXORrr},   {X86::PANDNrr, X86::VPANDNrr},
    {X86::ANDPSrr, X86::VANDPSrr}, {X86::ORPSrr, X86::VORPSrr},
    {X86::XORPSrr, X86::VXORPSrr}, {X86::ANDPDrr, X86::VANDPDrr},
    {X86::ORPDrr, X86::VORPDrr},   {X86::XORPDrr, X86::VXORPDrr},
};

char AVXLogicCombinerPass::ID = 0;
} // namespace

static RegisterPass<AVXLogicCombinerPass>
    X("x86-logic-opt", "X86 Logical Operations Chain Optimizer", false, false);