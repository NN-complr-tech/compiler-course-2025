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
using namespace llvm;

namespace {
class ExamplePass : public MachineFunctionPass {
public:
  static char ID;
  ExamplePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

char ExamplePass::ID = 0;

static bool isAddInstr(MachineInstr &MI) {
  return MI.getOpcode() == X86::ADDPDrr || MI.getOpcode() == X86::ADDPDrm;
}

static bool isMulInstr(MachineInstr &MI) {
  return MI.getOpcode() == X86::MULPDrr || MI.getOpcode() == X86::MULPDrm;
}

static bool existInvector(std::vector<MachineInstr *> *vector,
                          MachineInstr *val) {
  return std::find(vector->begin(), vector->end(), val) == vector->end();
}

bool ExamplePass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = STI.getInstrInfo();
  const TargetRegisterInfo *TRI = &TII->getRegisterInfo();

  std::vector<MachineInstr *> MIvector;

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      if (!(isMulInstr(MI)))
        continue;

      auto &op = MI.getOperand(0);
      for (auto &secondMI : MBB) {
        int ind = secondMI.findRegisterUseOperandIdx(op.getReg(), TRI, false);
        if (ind != -1 && isAddInstr(secondMI)) {
          if (ind == 0)
            break;
          int second_ind = 3 - ind;

          MIMetadata MIMD(MI);
          MachineInstr *Mul = &MI;
          MachineInstr *Add = &secondMI;

          BuildMI(MBB, Mul, MIMD, TII->get(X86::VFMADD213PDr),
                  Add->getOperand(0).getReg())
              .addReg(Mul->getOperand(1).getReg())
              .addReg(Mul->getOperand(2).getReg())
              .addReg(Add->getOperand(second_ind).getReg());

          if (existInvector(&MIvector, Mul))
            MIvector.push_back(Mul);
          if (existInvector(&MIvector, Add))
            MIvector.push_back(Add);
        }
      }
    }
  }

  for (auto &MI : MIvector) {
    MI->eraseFromParent();
  }

  return true;
}

static llvm::RegisterPass<ExamplePass>
    X("fma-decompose-x86",
      "Decomposes single FMA instructions into MUL and ADD ones", false, false);