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
class X86LogicOptimizer : public MachineFunctionPass {
  const X86InstrInfo *TII = nullptr;
  MachineRegisterInfo *MRI = nullptr;

  static const DenseMap<unsigned, unsigned> OpcodeMapping;

  struct LogicOpChain {
    MachineInstr *FirstInstr;
    MachineInstr *SecondInstr;
    unsigned FirstOpcode;
    unsigned SecondOpcode;
    Register FirstDest;
    Register SecondDest;
  };

  bool findLogicChain(MachineInstr &MI, LogicOpChain &Chain) {
    if (MI.getNumOperands() < 3 || !MI.getOperand(1).isReg())
      return false;

    Register SrcReg = MI.getOperand(1).getReg();
    if (!MRI->hasOneUse(SrcReg))
      return false;

    MachineInstr *DefMI = MRI->getUniqueVRegDef(SrcReg);
    if (!DefMI || DefMI->getNumOperands() < 3)
      return false;

    unsigned FirstOp = DefMI->getOpcode();
    unsigned SecondOp = MI.getOpcode();

    if (!OpcodeMapping.count(FirstOp) || !OpcodeMapping.count(SecondOp))
      return false;

    Chain.FirstInstr = DefMI;
    Chain.SecondInstr = &MI;
    Chain.FirstOpcode = FirstOp;
    Chain.SecondOpcode = SecondOp;
    Chain.FirstDest = DefMI->getOperand(0).getReg();
    Chain.SecondDest = MI.getOperand(0).getReg();
    return true;
  }

  bool optimizeDistributiveLaw(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator &It,
                               const LogicOpChain &Chain) {
    return false;
  }

  bool optimizeCommonPatterns(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator &It,
                              const LogicOpChain &Chain) {
    MachineInstr &FirstMI = *Chain.FirstInstr;
    MachineInstr &SecondMI = *Chain.SecondInstr;

    Register FirstSrc1 = FirstMI.getOperand(1).getReg();
    Register FirstSrc2 = FirstMI.getOperand(2).getReg();
    Register SecondSrc2 = SecondMI.getOperand(2).getReg();

    if ((Chain.FirstOpcode == X86::ANDPSrr ||
         Chain.FirstOpcode == X86::PANDrr ||
         Chain.FirstOpcode == X86::ANDPDrr) &&
        (Chain.SecondOpcode == X86::ORPSrr ||
         Chain.SecondOpcode == X86::PORrr ||
         Chain.SecondOpcode == X86::ORPDrr)) {

      if (SecondSrc2 == FirstSrc1 || SecondSrc2 == FirstSrc2) {
        Register SourceReg = (SecondSrc2 == FirstSrc1) ? FirstSrc1 : FirstSrc2;
        BuildMI(MBB, It, SecondMI.getDebugLoc(), TII->get(X86::MOVAPSrr),
                SecondMI.getOperand(0).getReg())
            .addReg(SourceReg);
        return true;
      }
    }

    if ((Chain.SecondOpcode == X86::ANDPSrr ||
         Chain.SecondOpcode == X86::PANDrr ||
         Chain.SecondOpcode == X86::ANDPDrr) &&
        (Chain.FirstOpcode == X86::ORPSrr || Chain.FirstOpcode == X86::PORrr ||
         Chain.FirstOpcode == X86::ORPDrr)) {

      Register SecondSrc1 = SecondMI.getOperand(1).getReg();
      if (SecondSrc1 == FirstSrc1 || SecondSrc1 == FirstSrc2) {
        BuildMI(MBB, It, SecondMI.getDebugLoc(), TII->get(X86::MOVAPSrr),
                SecondMI.getOperand(0).getReg())
            .addReg(FirstMI.getOperand(0).getReg());
        return true;
      }
    }

    return false;
  }

  void mergeToAVXInstructions(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator &It,
                              const LogicOpChain &Chain) {
    MachineInstr &FirstMI = *Chain.FirstInstr;
    MachineInstr &SecondMI = *Chain.SecondInstr;

    Register TempReg = MRI->createVirtualRegister(
        MRI->getRegClass(FirstMI.getOperand(0).getReg()));

    BuildMI(MBB, It, FirstMI.getDebugLoc(),
            TII->get(OpcodeMapping.at(Chain.FirstOpcode)), TempReg)
        .addReg(FirstMI.getOperand(1).getReg())
        .addReg(FirstMI.getOperand(2).getReg());

    BuildMI(MBB, It, SecondMI.getDebugLoc(),
            TII->get(OpcodeMapping.at(Chain.SecondOpcode)),
            SecondMI.getOperand(0).getReg())
        .addReg(TempReg)
        .addReg(SecondMI.getOperand(2).getReg());
  }

  bool processInstruction(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator &It) {
    MachineInstr &CurrentMI = *It;

    if (!OpcodeMapping.count(CurrentMI.getOpcode()))
      return false;

    LogicOpChain Chain;

    if (findLogicChain(CurrentMI, Chain)) {
      if (optimizeCommonPatterns(MBB, It, Chain)) {
        Chain.FirstInstr->eraseFromParent();
        It = MBB.erase(It);
        return true;
      }

      mergeToAVXInstructions(MBB, It, Chain);
      Chain.FirstInstr->eraseFromParent();
      It = MBB.erase(It);
      return true;
    }

    upgradeSingleInstruction(MBB, It);
    return true;
  }

  void upgradeSingleInstruction(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator &It) {
    MachineInstr &MI = *It;
    unsigned NewOpc = OpcodeMapping.lookup(MI.getOpcode());

    BuildMI(MBB, It, MI.getDebugLoc(), TII->get(NewOpc),
            MI.getOperand(0).getReg())
        .addReg(MI.getOperand(1).getReg())
        .addReg(MI.getOperand(2).getReg());

    It = MBB.erase(It);
  }

public:
  static char ID;
  X86LogicOptimizer() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "X86 Logical Operations Optimizer";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    TII = ST.getInstrInfo();
    MRI = &MF.getRegInfo();

    bool Modified = false;

    for (auto &MBB : MF) {
      auto It = MBB.begin();
      while (It != MBB.end()) {
        if (OpcodeMapping.count(It->getOpcode())) {
          auto CurrentIt = It;
          ++It;

          if (processInstruction(MBB, CurrentIt)) {
            Modified = true;
          }
        } else {
          ++It;
        }
      }
    }

    return Modified;
  }
};

const DenseMap<unsigned, unsigned> X86LogicOptimizer::OpcodeMapping = {
    {X86::PANDrr, X86::VPANDrr},   {X86::PORrr, X86::VPORrr},
    {X86::PXORrr, X86::VPXORrr},   {X86::PANDNrr, X86::VPANDNrr},

    {X86::ANDPSrr, X86::VANDPSrr}, {X86::ORPSrr, X86::VORPSrr},
    {X86::XORPSrr, X86::VXORPSrr},

    {X86::ANDPDrr, X86::VANDPDrr}, {X86::ORPDrr, X86::VORPDrr},
    {X86::XORPDrr, X86::VXORPDrr},
};

char X86LogicOptimizer::ID = 0;

} // namespace

static RegisterPass<X86LogicOptimizer>
    Y("x86-logic-combine", "X86 Logical Operations Combination Optimizer",
      false, false);