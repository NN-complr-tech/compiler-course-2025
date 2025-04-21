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
#include "llvm/Passes/PassPlugin.h"

#define DEBUG_TYPE "combine-logic-ops"

namespace {

class CombineLogicOpsPass : public llvm::MachineFunctionPass {
public:
  static char ID;
  CombineLogicOpsPass() : llvm::MachineFunctionPass(ID) {}
  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &ST = MF.getSubtarget<llvm::X86Subtarget>();
    if (!ST.hasAVX()) // https://www.youtube.com/watch?v=1IAwkEdRZZw
      return false;
    const llvm::X86InstrInfo *TII = ST.getInstrInfo();
    llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;
    // pass 1: combine chains of two logical ops
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> ToErase;
      for (auto &MI2 : MBB) {
        unsigned Opc2 = MI2.getOpcode();
        if (Opc2 == llvm::X86::PORrr || Opc2 == llvm::X86::PANDrr ||
            Opc2 == llvm::X86::PXORrr) {
          llvm::Register Mid = MI2.getOperand(1).getReg();
          auto *MI1 = MRI.getUniqueVRegDef(Mid);
          if (!MI1)
            continue;
          unsigned Opc1 = MI1->getOpcode();
          if (!(Opc1 == llvm::X86::PORrr || Opc1 == llvm::X86::PANDrr ||
                Opc1 == llvm::X86::PXORrr))
            continue;
          if (!MRI.hasOneUse(Mid))
            continue;
          auto getVP = [&](unsigned O) {
            if (O == llvm::X86::PORrr)
              return llvm::X86::VPORrr;
            if (O == llvm::X86::PANDrr)
              return llvm::X86::VPANDrr;
            return llvm::X86::VPXORrr;
          };
          unsigned VP1 = getVP(Opc1), VP2 = getVP(Opc2);
          llvm::Register A = MI1->getOperand(1).getReg();
          llvm::Register B = MI1->getOperand(2).getReg();
          llvm::Register C = MI2.getOperand(2).getReg();
          llvm::Register Dst = MI2.getOperand(0).getReg();
          llvm::BuildMI(MBB, *MI1, MI1->getDebugLoc(), TII->get(VP1), Mid)
              .addReg(A)
              .addReg(B);
          llvm::BuildMI(MBB, MI2, MI2.getDebugLoc(), TII->get(VP2), Dst)
              .addReg(Mid)
              .addReg(C);
          ToErase.push_back(MI1);
          ToErase.push_back(&MI2);
          Changed = true;
        }
      }
      for (auto *Old : ToErase)
        Old->eraseFromParent();
    }

    // pass 2: simplifying logical idioms
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;
      for (auto &MI : MBB) {
        unsigned Opc = MI.getOpcode();
        // XOR x,x -> zero via VPXORrr
        if (Opc == llvm::X86::PXORrr) {
          auto &o1 = MI.getOperand(1), &o2 = MI.getOperand(2);
          if (o1.isReg() && o2.isReg() && o1.getReg() == o2.getReg()) {
            unsigned VPX = llvm::X86::VPXORrr;
            llvm::Register D = MI.getOperand(0).getReg();
            llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(VPX), D)
                .addReg(D)
                .addReg(D);
            ToErase.push_back(&MI);
            Changed = true;
          }
        }
        // OR x,x or AND x,x -> copy
        if (Opc == llvm::X86::PORrr || Opc == llvm::X86::PANDrr) {
          auto &o1 = MI.getOperand(1), &o2 = MI.getOperand(2);
          if (o1.isReg() && o2.isReg() && o1.getReg() == o2.getReg()) {
            llvm::BuildMI(MBB, MI, MI.getDebugLoc(),
                          TII->get(llvm::TargetOpcode::COPY),
                          MI.getOperand(0).getReg())
                .addReg(o1.getReg());
            ToErase.push_back(&MI);
            Changed = true;
          }
        }
      }
      for (auto *Old : ToErase)
        Old->eraseFromParent();
    }

    // PASS 3: AND(X, 0) -> VPXORrr X,X
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;
      for (auto &MI : MBB) {
        if (MI.getOpcode() == llvm::X86::PANDrr) {
          auto &r1 = MI.getOperand(1), &r2 = MI.getOperand(2);
          auto *Def = MRI.getUniqueVRegDef(r2.getReg());
          if (!Def || Def->getOpcode() != llvm::X86::PXORrr)
            continue;
          auto &d1 = Def->getOperand(1), &d2 = Def->getOperand(2);
          if (!d1.isReg() || !d2.isReg() || d1.getReg() != d2.getReg())
            continue;
          // VPXORrr X, X, X
          unsigned VPX = llvm::X86::VPXORrr;
          llvm::Register D = MI.getOperand(0).getReg();
          llvm::BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(VPX), D)
              .addReg(D)
              .addReg(D);
          ToErase.push_back(Def);
          ToErase.push_back(&MI);
          Changed = true;
        }
      }
      for (auto *Old : ToErase)
        Old->eraseFromParent();
    }

    // pass 4: single ops to AVX
    for (auto &MBB : MF) {
      for (auto MI = MBB.begin(), ME = MBB.end(); MI != ME;) {
        llvm::MachineInstr &Instr = *MI++;
        unsigned Opc = Instr.getOpcode();
        unsigned NewOpc = 0;
        switch (Opc) {
        case llvm::X86::PANDrr:
          NewOpc = llvm::X86::VPANDrr;
          break;
        case llvm::X86::PORrr:
          NewOpc = llvm::X86::VPORrr;
          break;
        case llvm::X86::PXORrr:
          NewOpc = llvm::X86::VPXORrr;
          break;
        case llvm::X86::PANDNrr:
          NewOpc = llvm::X86::VPANDNrr;
          break;
        default:
          break;
        }
        if (NewOpc) {
          llvm::Register D = Instr.getOperand(0).getReg();
          llvm::Register S1 = Instr.getOperand(1).getReg();
          llvm::Register S2 = Instr.getOperand(2).getReg();
          llvm::BuildMI(MBB, Instr, Instr.getDebugLoc(), TII->get(NewOpc), D)
              .addReg(S1)
              .addReg(S2);
          Instr.eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed;
  }
};

char CombineLogicOpsPass::ID = 0;

} // namespace

static llvm::RegisterPass<CombineLogicOpsPass>
    X("combine-logic-ops-x86", "Combine logic sequences into AVX", false,
      false);
