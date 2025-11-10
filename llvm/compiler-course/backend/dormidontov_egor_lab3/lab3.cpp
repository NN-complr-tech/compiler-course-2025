#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct FmaPattern {
  unsigned MulOp, SubOp, AddOp, XorOp, VFMSUB213, VFNMADD213;
  const char *Name;
};

static const FmaPattern Patterns[] = {
    // Packed single-precision (SSE + AVX)
    {X86::MULPSrr, X86::SUBPSrr, X86::ADDPSrr, X86::XORPSrr, X86::VFMSUB213PSr,
     X86::VFNMADD213PSr, "PS"},
    {X86::VMULPSrr, X86::VSUBPSrr, X86::VADDPSrr, X86::XORPSrr,
     X86::VFMSUB213PSr, X86::VFNMADD213PSr, "VPS"},

    // Packed double-precision (SSE2 + AVX)
    {X86::MULPDrr, X86::SUBPDrr, X86::ADDPDrr, X86::XORPDrr, X86::VFMSUB213PDr,
     X86::VFNMADD213PDr, "PD"},
    {X86::VMULPDrr, X86::VSUBPDrr, X86::VADDPDrr, X86::XORPDrr,
     X86::VFMSUB213PDr, X86::VFNMADD213PDr, "VPD"},

    // Scalar single-precision (SSE + AVX)
    {X86::MULSSrr, X86::SUBSSrr, X86::ADDSSrr, X86::XORPSrr, X86::VFMSUB213SSr,
     X86::VFNMADD213SSr, "SS"},
    {X86::VMULSSrr, X86::VSUBSSrr, X86::VADDSSrr, X86::XORPSrr,
     X86::VFMSUB213SSr, X86::VFNMADD213SSr, "VSS"},

    // Scalar double-precision (SSE2 + AVX)
    {X86::MULSDrr, X86::SUBSDrr, X86::ADDSDrr, X86::XORPDrr, X86::VFMSUB213SDr,
     X86::VFNMADD213SDr, "SD"},
    {X86::VMULSDrr, X86::VSUBSDrr, X86::VADDSDrr, X86::XORPDrr,
     X86::VFMSUB213SDr, X86::VFNMADD213SDr, "VSD"},
};

class FmsubOptPass : public MachineFunctionPass {
public:
  static char ID;
  FmsubOptPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char FmsubOptPass::ID = 0;

bool FmsubOptPass::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  const X86InstrInfo *TII =
      static_cast<const X86InstrInfo *>(MF.getSubtarget().getInstrInfo());

  for (auto &MBB : MF) {
    for (auto MI = MBB.begin(); MI != MBB.end();) {
      bool PatternMatched = false;

      for (const auto &P : Patterns) {
        // (a * b) - c → VFMSUB213
        if (MI->getOpcode() == P.SubOp && MI != MBB.begin()) {
          auto MulMI = std::prev(MI);
          if (MulMI->getOpcode() == P.MulOp) {
            auto &MulDst = MulMI->getOperand(0);
            auto &MulA = MulMI->getOperand(1);
            auto &MulB = MulMI->getOperand(2);
            auto &SubDst = MI->getOperand(0);
            auto &SubA = MI->getOperand(1);
            auto &SubB = MI->getOperand(2);

            if (SubA.isReg() && SubA.getReg() == MulDst.getReg() &&
                SubDst.isReg()) {
              DebugLoc DL = MI->getDebugLoc();
              BuildMI(MBB, MI, DL, TII->get(P.VFMSUB213), SubDst.getReg())
                  .add(MulA)
                  .add(MulB)
                  .add(SubB);
              MI = MBB.erase(MI);
              MBB.erase(MulMI);
              Changed = true;
              PatternMatched = true;
              break;
            }
          }
        }
        // c - (a * b) → VFNMADD213
        if (MI->getOpcode() == P.SubOp && MI != MBB.begin()) {
          auto MulMI = std::prev(MI);
          if (MulMI->getOpcode() == P.MulOp) {
            auto &MulDst = MulMI->getOperand(0);
            auto &MulA = MulMI->getOperand(1);
            auto &MulB = MulMI->getOperand(2);
            auto &SubDst = MI->getOperand(0);
            auto &SubA = MI->getOperand(1);
            auto &SubB = MI->getOperand(2);

            if (SubB.isReg() && SubB.getReg() == MulDst.getReg() &&
                SubDst.isReg()) {
              DebugLoc DL = MI->getDebugLoc();
              BuildMI(MBB, MI, DL, TII->get(P.VFNMADD213), SubDst.getReg())
                  .add(MulA)
                  .add(MulB)
                  .add(SubA);
              MI = MBB.erase(MI);
              MBB.erase(MulMI);
              Changed = true;
              PatternMatched = true;
              break;
            }
          }
        }
        // -(a * b) + c → VFNMADD213 (через XOR/NEG)
        if (MI->getOpcode() == P.AddOp && MI != MBB.begin()) {
          auto XorMI = std::prev(MI);
          if (XorMI->getOpcode() == P.XorOp && XorMI != MBB.begin()) {
            auto MulMI = std::prev(XorMI);
            if (MulMI->getOpcode() == P.MulOp) {
              auto &MulDst = MulMI->getOperand(0);
              auto &MulA = MulMI->getOperand(1);
              auto &MulB = MulMI->getOperand(2);

              auto &XorDst = XorMI->getOperand(0);

              auto &AddDst = MI->getOperand(0);
              auto &AddA = MI->getOperand(1);
              auto &AddB = MI->getOperand(2);
              if (XorDst.isReg() && MulDst.isReg() &&
                  XorDst.getReg() == MulDst.getReg() &&
                  ((AddA.isReg() && AddA.getReg() == XorDst.getReg()) ||
                   (AddB.isReg() && AddB.getReg() == XorDst.getReg()))) {
                Register cReg = (AddA.getReg() == XorDst.getReg())
                                    ? AddB.getReg()
                                    : AddA.getReg();
                DebugLoc DL = MI->getDebugLoc();
                BuildMI(MBB, MI, DL, TII->get(P.VFNMADD213), AddDst.getReg())
                    .add(MulA)
                    .add(MulB)
                    .addReg(cReg);
                MI = MBB.erase(MI);
                MBB.erase(XorMI);
                MBB.erase(MulMI);
                Changed = true;
                PatternMatched = true;
                break;
              }
            }
          }
        }
      }
      if (!PatternMatched)
        ++MI;
    }
  }
  return Changed;
}

} // namespace

static RegisterPass<FmsubOptPass>
    X("fmsub-opt",
      "Fused multiply-subtract/add for X86 (VFMSUB/VFNMADD) (SSE+AVX+FMA)",
      false, false);
