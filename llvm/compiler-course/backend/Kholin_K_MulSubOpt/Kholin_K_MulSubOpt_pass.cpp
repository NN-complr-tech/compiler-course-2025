#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mul-sub-opt"

namespace {
class MulSubOpt : public MachineFunctionPass {
public:
  static char ID;
  MulSubOpt() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();

    LLVM_DEBUG({
      dbgs() << "\n***************************************************\n"
             << "* Running MulSubOpt on function: " << MF.getName() << "\n"
             << "***************************************************\n";
    });

    if (!ST.hasFMA()) {
      LLVM_DEBUG(dbgs() << "FMA not supported, skipping pass\n");
      return false;
    }

    bool Changed = false;
    SmallVector<MachineInstr *, 16> ToRemove;

    for (auto &MBB : MF) {
      LLVM_DEBUG(dbgs() << "\nProcessing BB: " << MBB.getName() << "\n");

      for (auto &MI : MBB) {
        LLVM_DEBUG({
          dbgs() << "\nInspecting instruction: ";
          MI.print(dbgs());
          dbgs() << "  Opcode: " << MI.getOpcode() << "\n";
        });

        // Handle classic SUB pattern
        if (isSubtractionOpcode(MI.getOpcode())) {
          if (MI.getNumOperands() < 3 || !MI.getOperand(0).isReg()) {
            LLVM_DEBUG(dbgs() << "  Doesn't have 3 operands or doesn't define reg, skipping\n");
            continue;
          }

          LLVM_DEBUG(dbgs() << "  Found SUB instruction!\n");

          Register SubReg = MI.getOperand(0).getReg();
          Register Op2Reg = MI.getOperand(2).getReg();

          MachineInstr *MulMI = MRI.getUniqueVRegDef(Op2Reg);
          while (MulMI && MulMI->isCopy()) {
            Register SrcReg = MulMI->getOperand(1).getReg();
            if (!SrcReg.isVirtual()) break;
            MulMI = MRI.getUniqueVRegDef(SrcReg);
          }

          if (!MulMI || !isMultiplicationOpcode(MulMI->getOpcode())) {
            LLVM_DEBUG(dbgs() << "  MUL instruction not found or invalid\n");
            continue;
          }

          LLVM_DEBUG({
            dbgs() << "  Found MUL instruction:\n";
            MulMI->print(dbgs());
          });

          unsigned FMAOpcode = getFMAOpcode(MI.getOpcode());
          if (!FMAOpcode) {
            LLVM_DEBUG(dbgs() << "  No FMA opcode for SUB opcode: " << MI.getOpcode() << "\n");
            continue;
          }

          LLVM_DEBUG(dbgs() << "  Creating FMA instruction with opcode: " << FMAOpcode << "\n");
          MachineInstrBuilder MIB =
              BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(FMAOpcode), SubReg);

          MIB.add(MulMI->getOperand(1));
          MIB.add(MulMI->getOperand(2));
          MIB.add(MI.getOperand(1));

          LLVM_DEBUG({
            dbgs() << "  Created new FMA instruction:\n";
            MIB->print(dbgs());
          });

          ToRemove.push_back(MulMI);
          ToRemove.push_back(&MI);
          Changed = true;
        }
        // Handle negated pattern: MULSS + XOR + ADDSS
        else if (MI.getOpcode() == X86::ADDSSrr || MI.getOpcode() == X86::ADDSDrr) {
          LLVM_DEBUG(dbgs() << "  Checking for MUL + XOR + ADD pattern\n");

          if (MI.getNumOperands() < 3 || !MI.getOperand(0).isReg()) {
            LLVM_DEBUG(dbgs() << "  Doesn't have 3 operands or doesn't define reg, skipping\n");
            continue;
          }

          for (unsigned OpIdx = 1; OpIdx <= 2; ++OpIdx) {
            Register OpReg = MI.getOperand(OpIdx).getReg();
            LLVM_DEBUG(dbgs() << "  Checking operand " << OpIdx << " (reg: " << OpReg << ")\n");

            MachineInstr *Mov2MI = MRI.getUniqueVRegDef(OpReg);
            if (!Mov2MI || Mov2MI->getOpcode() != X86::MOVDI2SSrr) {
              LLVM_DEBUG(dbgs() << "  No MOVDI2SS found for operand " << OpIdx << "\n");
              continue;
            }

            MachineInstr *XorMI = MRI.getUniqueVRegDef(Mov2MI->getOperand(1).getReg());
            if (!XorMI || XorMI->getOpcode() != X86::XOR32ri) {
              LLVM_DEBUG(dbgs() << "  No XOR32ri found for operand " << OpIdx << "\n");
              continue;
            }

            if (XorMI->getOperand(2).getImm() != 0x80000000) {
              LLVM_DEBUG(dbgs() << "  XOR has wrong immediate value: " 
                               << XorMI->getOperand(2).getImm() << "\n");
              continue;
            }

            MachineInstr *Mov1MI = MRI.getUniqueVRegDef(XorMI->getOperand(1).getReg());
            if (!Mov1MI || Mov1MI->getOpcode() != X86::MOVSS2DIrr) {
              LLVM_DEBUG(dbgs() << "  No MOVSS2DIrr found for operand " << OpIdx << "\n");
              continue;
            }

            MachineInstr *MulMI = MRI.getUniqueVRegDef(Mov1MI->getOperand(1).getReg());
            if (!MulMI || !isMultiplicationOpcode(MulMI->getOpcode())) {
              LLVM_DEBUG(dbgs() << "  MUL instruction not found or invalid\n");
              continue;
            }

            LLVM_DEBUG(dbgs() << "  Found complete MUL + XOR + ADD pattern!\n");
    
            Register OtherOpReg = MI.getOperand(3 - OpIdx).getReg();
            Register DstReg = MI.getOperand(0).getReg();

            unsigned FMAOpcode = getNegatedFMAOpcode(MI.getOpcode());
            if (!FMAOpcode) {
              LLVM_DEBUG(dbgs() << "  No FMA opcode for ADD opcode: " << MI.getOpcode() << "\n");
              continue;
            }

            LLVM_DEBUG(dbgs() << "  Creating FMA instruction with opcode: " << FMAOpcode << "\n");
            MachineInstrBuilder MIB =
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(FMAOpcode), DstReg);

            MIB.add(MulMI->getOperand(1));
            MIB.add(MulMI->getOperand(2));
            MIB.addReg(OtherOpReg);

            LLVM_DEBUG({
              dbgs() << "  Created new FMA instruction:\n";
              MIB->print(dbgs());
            });

            ToRemove.push_back(MulMI);
            ToRemove.push_back(Mov1MI);
            ToRemove.push_back(XorMI);
            ToRemove.push_back(Mov2MI);
            ToRemove.push_back(&MI);
            Changed = true;
            break;
          }
        }
      }
    }

    for (auto *MI : ToRemove) {
      if (MI->getParent()) {
        LLVM_DEBUG({
          dbgs() << "Removing instruction: ";
          MI->print(dbgs());
        });
        MI->eraseFromParent();
      }
    }

    if (Changed) {
      LLVM_DEBUG(dbgs() << "\nFunction was modified by MulSubOpt\n");
    } else {
      LLVM_DEBUG(dbgs() << "\nNo changes made to function by MulSubOpt\n");
    }

    return Changed;
  }

  StringRef getPassName() const override {
    return "Fused Multiply-Subtract Optimization";
  }

private:
  unsigned getFMAOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::SUBSSrr: case X86::VSUBSSrr: case X86::SUBSSrr_Int: case X86::VSUBSSrr_Int:
      return X86::VFNMADD213SSr;
    case X86::SUBSDrr: case X86::VSUBSDrr: case X86::SUBSDrr_Int: case X86::VSUBSDrr_Int:
      return X86::VFNMADD213SDr;
    default:
      return 0;
    }
  }

  unsigned getNegatedFMAOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::ADDSSrr: case X86::VADDSSrr:
      return X86::VFNMADD213SSr;
    case X86::ADDSDrr: case X86::VADDSDrr:
      return X86::VFNMADD213SDr;
    default:
      return 0;
    }
  }

  bool isSubtractionOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::SUBSSrr: case X86::SUBSDrr:
    case X86::VSUBSSrr: case X86::VSUBSDrr:
    case X86::SUBSSrr_Int: case X86::SUBSDrr_Int:
    case X86::VSUBSSrr_Int: case X86::VSUBSDrr_Int:
      return true;
    default:
      return false;
    }
  }

  bool isMultiplicationOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::MULSSrr: case X86::MULSDrr:
    case X86::VMULSSrr: case X86::VMULSDrr:
    case X86::MULSSrr_Int: case X86::MULSDrr_Int:
    case X86::VMULSSrr_Int: case X86::VMULSDrr_Int:
      return true;
    default:
      return false;
    }
  }
};

char MulSubOpt::ID = 0;
} // end anonymous namespace

static llvm::RegisterPass<MulSubOpt> X("mul-sub-opt", "MulSubOpt MIR", false, false);
