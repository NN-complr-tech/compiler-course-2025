#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class FMADecomposePass : public llvm::MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &STI = MF.getSubtarget<llvm::X86Subtarget>();
    const llvm::X86InstrInfo *TII = STI.getInstrInfo();
    bool status = false;
    llvm::SmallVector<llvm::MachineInstr *, 8> WorkList;

    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        unsigned opcode = MI.getOpcode();
        llvm::StringRef Name = TII->getName(opcode);
        if (Name.starts_with("VFMADD") && !Name.contains("SUB")) {
          WorkList.push_back(&MI);
        }
      }
    }
    for (auto *MI : WorkList) {
      unsigned multype = 0;
      unsigned addtype = 0;

      llvm::MachineBasicBlock &MBB = *MI->getParent();
      llvm::MachineOperand &DST = MI->getOperand(0);
      llvm::MachineOperand &SRC1 = MI->getOperand(1);
      llvm::MachineOperand &SRC2 = MI->getOperand(2);
      llvm::MachineOperand &SRC3 = MI->getOperand(3);

      llvm::StringRef InstrName = TII->getName(MI->getOpcode());
      llvm::MachineOperand *Mul1 = nullptr;
      llvm::MachineOperand *Mul2 = nullptr;
      llvm::MachineOperand *Add = nullptr;

      if (InstrName.contains("132")) {
        Mul1 = &SRC1;
        Mul2 = &SRC2;
        Add = &SRC3;
      } else if (InstrName.contains("213")) {
        Mul1 = &SRC2;
        Mul2 = &SRC1;
        Add = &SRC3;
      } else if (InstrName.contains("231")) {
        Mul1 = &SRC2;
        Mul2 = &SRC3;
        Add = &SRC1;
      } else {
        continue;
      }
      if (InstrName.contains("PS")) {
        multype = llvm::X86::VMULPSrr;
        addtype = llvm::X86::VADDPSrr;
      } else if (InstrName.contains("PD")) {
        multype = llvm::X86::VMULPDrr;
        addtype = llvm::X86::VADDPDrr;
      } else if (InstrName.contains("SS")) {
        multype = llvm::X86::VMULSSrr;
        addtype = llvm::X86::VADDSSrr;
      } else if (InstrName.contains("SD")) {
        multype = llvm::X86::VMULSDrr;
        addtype = llvm::X86::VADDSDrr;
      } else {
        continue;
      }

      llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
      llvm::Register TmpMul =
          MRI.createVirtualRegister(MRI.getRegClass(SRC1.getReg()));
      llvm::BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(multype), TmpMul)
          .addReg(Mul1->getReg())
          .addReg(Mul2->getReg());
      llvm::BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(addtype), DST.getReg())
          .addReg(Add->getReg())
          .addReg(TmpMul);
      MI->eraseFromParent();
      status = true;
    }
    return status;
  }
};

char FMADecomposePass::ID = 0;
} // namespace

static llvm::RegisterPass<FMADecomposePass>
    X("fma-decompose", "Decompose FMA into MUL + ADD", false, false);
