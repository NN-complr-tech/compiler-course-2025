#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace {

class AVXLogicOptPass : public llvm::MachineFunctionPass {
public:
  static char ID;
  AVXLogicOptPass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const llvm::X86Subtarget &ST = MF.getSubtarget<llvm::X86Subtarget>();
    if (!ST.hasAVX())
      return false;

    const llvm::X86InstrInfo *TII = ST.getInstrInfo();
    llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
    const llvm::TargetRegisterInfo *TRI = ST.getRegisterInfo();
    bool Changed = false;

    // Pass 1: Simplify redundant logic ops
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;
      for (auto &MI : MBB) {
        unsigned Opc = MI.getOpcode();
        if ((Opc == llvm::X86::PORrr || Opc == llvm::X86::PANDrr) &&
            MI.getOperand(1).getReg() == MI.getOperand(2).getReg()) {
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(llvm::TargetOpcode::COPY),
                  MI.getOperand(0).getReg())
              .addReg(MI.getOperand(1).getReg());
          ToErase.push_back(&MI);
        }
      }
      for (auto *MI : ToErase)
        MI->eraseFromParent();
      Changed |= !ToErase.empty();
    }

    // Pass 2: Optimize AND with zero
    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;
      for (auto &MI : MBB) {
        if (MI.getOpcode() == llvm::X86::PANDrr) {
          auto &Src = MI.getOperand(2);
          auto *Def = MRI.getUniqueVRegDef(Src.getReg());
          if (Def && Def->getOpcode() == llvm::X86::PXORrr &&
              Def->getOperand(1).getReg() == Def->getOperand(2).getReg()) {
            llvm::Register Dst = MI.getOperand(0).getReg();
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(llvm::X86::VPXORrr),
                    Dst)
                .addReg(Dst)
                .addReg(Dst);
            ToErase.push_back(Def);
            ToErase.push_back(&MI);
          }
        }
      }
      for (auto *MI : ToErase)
        MI->eraseFromParent();
      Changed |= !ToErase.empty();
    }

    // Pass 3: Upgrade to AVX
    const static llvm::DenseMap<unsigned, unsigned> AVXOpcodeMap = {
        {llvm::X86::PANDrr, llvm::X86::VPANDrr},
        {llvm::X86::PORrr, llvm::X86::VPORrr},
        {llvm::X86::PXORrr, llvm::X86::VPXORrr},
        {llvm::X86::PANDNrr, llvm::X86::VPANDNrr}};

    auto isVectorReg = [&](llvm::Register Reg) {
      if (!llvm::Register::isVirtualRegister(Reg))
        return false;
      const auto *RC = MRI.getRegClass(Reg);
      llvm::StringRef Name = TRI->getRegClassName(RC);
      return Name.starts_with("XMM") || Name.starts_with("YMM") ||
             Name.starts_with("ZMM") || Name.starts_with("VR");
    };

    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;
      for (auto &MI : MBB) {
        auto It = AVXOpcodeMap.find(MI.getOpcode());
        if (It == AVXOpcodeMap.end())
          continue;

        llvm::Register Src1 = MI.getOperand(1).getReg();
        llvm::Register Src2 = MI.getOperand(2).getReg();
        if (!isVectorReg(Src1) || !isVectorReg(Src2))
          continue;

        BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(It->second),
                MI.getOperand(0).getReg())
            .addReg(Src1)
            .addReg(Src2);
        ToErase.push_back(&MI);
      }
      for (auto *MI : ToErase)
        MI->eraseFromParent();
      Changed |= !ToErase.empty();
    }

    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 4> ToErase;

      for (auto MI = MBB.begin(); MI != MBB.end(); ++MI) {
        if (MI->getOpcode() == llvm::X86::VPANDrr) {
          llvm::MachineInstr *AndMI = &*MI;
          llvm::Register AndDest = AndMI->getOperand(0).getReg();

          if (!MRI.hasOneUse(AndDest))
            continue;

          auto NextMI = std::next(MI);
          if (NextMI == MBB.end() || NextMI->getOpcode() != llvm::X86::VPORrr)
            continue;

          llvm::Register A = AndMI->getOperand(1).getReg();
          llvm::Register B = AndMI->getOperand(2).getReg();
          llvm::Register C = NextMI->getOperand(2).getReg();

          BuildMI(MBB, *NextMI, NextMI->getDebugLoc(),
                  TII->get(llvm::X86::VPANDNrr), AndDest)
              .addReg(C)
              .addReg(B);

          BuildMI(MBB, *NextMI, NextMI->getDebugLoc(),
                  TII->get(llvm::X86::VPORrr), NextMI->getOperand(0).getReg())
              .addReg(AndDest)
              .addReg(A);

          ToErase.push_back(AndMI);
          ToErase.push_back(&*NextMI);
          ++MI;
        }
      }

      for (auto *MI : ToErase)
        MI->eraseFromParent();
      Changed |= !ToErase.empty();
    }

    return Changed;
  }
};

char AVXLogicOptPass::ID = 0;

} // namespace

static llvm::RegisterPass<AVXLogicOptPass>
    X("AVXLogicOptPass_Baranov_Aleksey_FIIT1_BACKEND",
      "AVX logic optimization pass", false, false);