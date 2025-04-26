#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class MulSubOpt : public MachineFunctionPass {
public:
  static char ID;
  MulSubOpt() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    const TargetRegisterInfo *TRI = ST.getRegisterInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();

    errs() << "\n***************************************************\n"
           << "* Running MulSubOpt on function: " << MF.getName() << "\n"
           << "***************************************************\n";

    if (!ST.hasFMA()) {
      errs() << "FMA not support, skipping pass\n";
      return false;
    }

    bool Changed = false;
    SmallVector<MachineInstr *, 16> ToRemove;

    for (auto &MBB : MF) {
      errs() << "\nProcessing BB: " << MBB.getName() << "\n";

      for (auto &MI : MBB) {
        errs() << "\nInspecting instruction: ";
        MI.print(errs());
        errs() << "  Opcode: " << MI.getOpcode() << "\n";

        if (!isSubtractionOpcode(MI.getOpcode())) {
          errs() << "  Not SUB instruction, skipping\n";
          continue;
        }

        if (MI.getNumOperands() < 3 || !MI.getOperand(0).isReg()) {
          errs() << "  Doesnt have 3 operands or doesnt define reg, skipping\n";
          continue;
        }

        errs() << "  Found SUB instruction!\n";

        Register SubReg = MI.getOperand(0).getReg();
        Register Op2Reg = MI.getOperand(2).getReg();

        MachineInstr *MulMI = nullptr;

        if (Op2Reg.isVirtual()) {
          MulMI = MRI.getUniqueVRegDef(Op2Reg);
          while (MulMI && MulMI->isCopy()) {
            Register SrcReg = MulMI->getOperand(1).getReg();
            if (!SrcReg.isVirtual())
              break;
            MulMI = MRI.getUniqueVRegDef(SrcReg);
          }
        } else {
          for (auto I = MachineBasicBlock::iterator(&MI), E = MBB.begin();
               I != E; --I) {
            if (I->definesRegister(Op2Reg, TRI)) {
              MulMI = &*I;
              break;
            }
          }
        }

        if (!MulMI || !isMultiplicationOpcode(MulMI->getOpcode())) {
          errs() << "  MUL instruction not found or invalid\n";
          continue;
        }

        errs() << "  Found MUL instruction:\n";
        MulMI->print(errs());

        unsigned FMAOpcode = getFMAOpcode(MI.getOpcode());
        if (!FMAOpcode) {
          errs() << "  No FMA opcode for SUB opcode: " << MI.getOpcode()
                 << "\n";
          continue;
        }

        errs() << "  Creating FMA instruction with opcode: " << FMAOpcode
               << "\n";
        MachineInstrBuilder MIB =
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(FMAOpcode), SubReg);

        MIB.add(MulMI->getOperand(1));
        MIB.add(MulMI->getOperand(2));
        MIB.add(MI.getOperand(1));

        for (const MachineOperand &MO : MI.implicit_operands()) {
          MIB.add(MO);
        }

        errs() << "  Created new FMA instruction:\n";
        MIB->print(errs());

        ToRemove.push_back(MulMI);
        ToRemove.push_back(&MI);
        Changed = true;
      }
    }

    for (auto *MI : ToRemove) {
      if (MI->getParent()) {
        errs() << "Removing instruction: ";
        MI->print(errs());
        MI->eraseFromParent();
      }
    }

    if (Changed) {
      errs() << "\nFunction was modified by MulSubOpt\n";
    } else {
      errs() << "\nNo changes made to function by MulSubOpt\n";
    }

    return Changed;
  }

  StringRef getPassName() const override {
    return "Fused Multiply-Subtract Optimization";
  }

private:
  unsigned getFMAOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::SUBSSrr:
    case X86::VSUBSSrr:
    case X86::SUBSSrr_Int:
    case X86::VSUBSSrr_Int:
      return X86::VFNMADD213SSr;
    case X86::SUBSDrr:
    case X86::VSUBSDrr:
    case X86::SUBSDrr_Int:
    case X86::VSUBSDrr_Int:
      return X86::VFNMADD213SDr;
    case X86::SUBSSrm:
    case X86::VSUBSSrm:
    case X86::SUBSSrm_Int:
    case X86::VSUBSSrm_Int:
      return X86::VFNMADD213SSm;
    case X86::SUBSDrm:
    case X86::VSUBSDrm:
    case X86::SUBSDrm_Int:
    case X86::VSUBSDrm_Int:
      return X86::VFNMADD213SDm;
    default:
      return 0;
    }
  }

  bool isSubtractionOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::SUBSSrr:
    case X86::SUBSDrr:
    case X86::VSUBSSrr:
    case X86::VSUBSDrr:
    case X86::SUBSSrm:
    case X86::SUBSDrm:
    case X86::VSUBSSrm:
    case X86::VSUBSDrm:
    case X86::SUBSSrr_Int:
    case X86::SUBSDrr_Int:
    case X86::VSUBSSrr_Int:
    case X86::VSUBSDrr_Int:
    case X86::SUBSSrm_Int:
    case X86::SUBSDrm_Int:
    case X86::VSUBSSrm_Int:
    case X86::VSUBSDrm_Int:
      return true;
    default:
      return false;
    }
  }

  bool isMultiplicationOpcode(unsigned Opcode) const {
    switch (Opcode) {
    case X86::MULSSrr:
    case X86::MULSDrr:
    case X86::VMULSSrr:
    case X86::VMULSDrr:
    case X86::MULSSrm:
    case X86::MULSDrm:
    case X86::VMULSSrm:
    case X86::VMULSDrm:
    case X86::MULSSrr_Int:
    case X86::MULSDrr_Int:
    case X86::VMULSSrr_Int:
    case X86::VMULSDrr_Int:
    case X86::MULSSrm_Int:
    case X86::MULSDrm_Int:
    case X86::VMULSSrm_Int:
    case X86::VMULSDrm_Int:
      return true;
    default:
      return false;
    }
  }
};

char MulSubOpt::ID = 0;
} // end anonymous namespace

static llvm::RegisterPass<MulSubOpt> X("mul-sub-opt", "MulSubOpt MIR", false,
                                       false);
