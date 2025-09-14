#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include <functional>

namespace {
class CombineLogicPass : public llvm::MachineFunctionPass {
private:
  bool Changed = false;
  const llvm::X86InstrInfo *TII = nullptr;
  const llvm::MachineRegisterInfo *MRI = nullptr;

  llvm::SmallVector<std::pair<unsigned, unsigned>, 8> LogicPairs;

  struct PendingReplace {
    llvm::MachineInstr *MI;
    unsigned NewOpcode;
  };

  void initMappings() {
    if (!LogicPairs.empty()) {
      return;
    }
    LogicPairs.push_back({llvm::X86::ANDPSrr, llvm::X86::VANDPSrr});
    LogicPairs.push_back({llvm::X86::ORPSrr, llvm::X86::VORPSrr});
    LogicPairs.push_back({llvm::X86::XORPSrr, llvm::X86::VXORPSrr});
    LogicPairs.push_back({llvm::X86::PANDrr, llvm::X86::VPANDrr});
    LogicPairs.push_back({llvm::X86::PORrr, llvm::X86::VPORrr});
    LogicPairs.push_back({llvm::X86::PXORrr, llvm::X86::VPXORrr});
    LogicPairs.push_back({llvm::X86::PANDNrr, llvm::X86::VPANDNrr});
  }

  unsigned findAVX(unsigned Opcode) const {
    for (auto &p : LogicPairs) {
      if (p.first == Opcode) {
        return p.second;
      }
    }
    return 0;
  }

  void optimizeLogicOperations(llvm::MachineFunction &MF) {
    llvm::SmallVector<PendingReplace, 32> Pending;

    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        unsigned NewOp = findAVX(MI.getOpcode());
        if (NewOp) {
          Pending.push_back({&MI, NewOp});
        }
      }
    }

    llvm::SmallVector<llvm::MachineInstr *, 32> ToErase;
    for (auto &PR : Pending) {
      llvm::MachineInstr *MI = PR.MI;
      unsigned NewOp = PR.NewOpcode;
      if (!MI || !MI->getParent()) {
        continue;
      }

      llvm::MachineBasicBlock &MBB = *MI->getParent();
      if (MI->getNumOperands() < 3) {
        continue;
      }
      llvm::Register DestReg = MI->getOperand(0).getReg();
      llvm::Register Src1 = MI->getOperand(1).getReg();
      llvm::Register Src2 = MI->getOperand(2).getReg();

      llvm::BuildMI(MBB, *MI, MI->getDebugLoc(), TII->get(NewOp), DestReg)
          .addReg(Src1)
          .addReg(Src2);

      ToErase.push_back(MI);
      Changed = true;
    }

    for (auto *I : ToErase) {
      if (I->isBundle()) {
        I->eraseFromParent();
      } else {
        I->eraseFromParent();
      }
    }
  }

  void combineLogicOperations(llvm::MachineFunction &MF) {
    llvm::SmallVector<llvm::MachineInstr *, 8> ToErase;
    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        unsigned SecondAVX = findAVX(MI.getOpcode());
        if (!SecondAVX) {
          continue;
        }

        if (MI.getNumOperands() < 2) {
          continue;
        }
        llvm::Register InterReg = MI.getOperand(1).getReg();
        if (!InterReg) {
          continue;
        }
        auto *FirstInstr = MRI->getUniqueVRegDef(InterReg);
        if (!FirstInstr) {
          continue;
        }
        if (!MRI->hasOneUse(InterReg)) {
          continue;
        }

        unsigned FirstOp = FirstInstr->getOpcode();
        unsigned FirstAVX = findAVX(FirstOp);
        if (!FirstAVX) {
          continue;
        }

        llvm::MachineBasicBlock &MBBFirst = *FirstInstr->getParent();
        llvm::MachineBasicBlock &MBBSecond = *MI.getParent();

        if (FirstInstr->getNumOperands() >= 3) {
          llvm::Register D1 = FirstInstr->getOperand(0).getReg();
          llvm::Register A1 = FirstInstr->getOperand(1).getReg();
          llvm::Register B1 = FirstInstr->getOperand(2).getReg();
          llvm::BuildMI(MBBFirst, *FirstInstr, FirstInstr->getDebugLoc(),
                        TII->get(FirstAVX), D1)
              .addReg(A1)
              .addReg(B1);
        }

        if (MI.getNumOperands() >= 3) {
          llvm::Register D2 = MI.getOperand(0).getReg();
          llvm::Register A2 = MI.getOperand(1).getReg();
          llvm::Register B2 = MI.getOperand(2).getReg();
          llvm::BuildMI(MBBSecond, MI, MI.getDebugLoc(), TII->get(SecondAVX),
                        D2)
              .addReg(A2)
              .addReg(B2);
        }

        ToErase.push_back(FirstInstr);
        ToErase.push_back(&MI);
        Changed = true;
      }
    }

    for (auto *I : ToErase) {
      I->eraseFromParent();
    }
  }

public:
  static char ID;
  CombineLogicPass() : llvm::MachineFunctionPass(ID) { initMappings(); }

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    Changed = false;
    const auto *ST = &MF.getSubtarget<llvm::X86Subtarget>();
    TII = ST->getInstrInfo();
    MRI = &MF.getRegInfo();

    optimizeLogicOperations(MF);
    combineLogicOperations(MF);

    return Changed;
  }
};

char CombineLogicPass::ID = 0;
} // namespace

static llvm::RegisterPass<CombineLogicPass>
    X("combine-logic-x86", "X86 Logical Operations Optimization Pass", false,
      false);