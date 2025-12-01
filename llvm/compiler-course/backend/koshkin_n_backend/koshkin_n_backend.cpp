#include "X86.h"
#include "X86InstrInfo.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"

#define DEBUG_TYPE "instrument-simd"

using namespace llvm;

namespace {

class InstrumentSimdPass : public MachineFunctionPass {
public:
  static char ID;
  InstrumentSimdPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Module *M = MF.getFunction().getParent();
    GlobalVariable *SimdCounter = M->getGlobalVariable("simd_counter");
    if (!SimdCounter) {
      Type *Int64Ty = Type::getInt64Ty(M->getContext());
      SimdCounter = new GlobalVariable(
          *M, Int64Ty, /*isConstant=*/false, GlobalValue::ExternalLinkage,
          ConstantInt::get(Int64Ty, 0), "simd_counter");
      SimdCounter->setAlignment(MaybeAlign(8));
    }

    const X86InstrInfo *TII =
        static_cast<const X86InstrInfo *>(MF.getSubtarget().getInstrInfo());

    bool Changed = false;

    static const std::vector<std::string> simd_opcodes = {
        // scalar float/double
        "ADDSSrm",
        "ADDSSrr",
        "ADDSDrm",
        "ADDSDrr",
        "SUBSSrm",
        "SUBSSrr",
        "SUBSDrm",
        "SUBSDrr",
        "MULSSrm",
        "MULSSrr",
        "MULSDrm",
        "MULSDrr",
        "DIVSSrm",
        "DIVSSrr",
        "DIVSDrm",
        "DIVSDrr",

        // packed float/double (SSE/AVX/AVX512)
        "ADDPSrm",
        "ADDPSrr",
        "ADDPSrm",
        "ADDPSrr",
        "ADDPSYrm",
        "ADDPSYrr",
        "ADDPDrm",
        "ADDPDrr",
        "ADDPDYrm",
        "ADDPDYrr",
        "ADDPDZ128rm",
        "ADDPDZ256rm",
        "ADDPDZrm",
        "VADDPSrm",
        "VADDPSrr",
        "VADDPDrm",
        "VADDPDrr",
        "VADDSSrm",
        "VADDSSrr",
        "VADDSDrm",
        "VADDSDrr",

        "SUBPSrm",
        "SUBPSrr",
        "SUBPSYrm",
        "SUBPSYrr",
        "SUBPDrm",
        "SUBPDrr",
        "SUBPDYrm",
        "SUBPDYrr",
        "VSUBPSrm",
        "VSUBPSrr",
        "VSUBPDrm",
        "VSUBPDrr",
        "VSUBSSrm",
        "VSUBSSrr",
        "VSUBSDrm",
        "VSUBSDrr",

        "MULPSrm",
        "MULPSrr",
        "MULPDrm",
        "MULPDrr",
        "VMULPSrm",
        "VMULPSrr",
        "VMULPDrm",
        "VMULPDrr",
        "VMULSSrm",
        "VMULSSrr",
        "VMULSDrm",
        "VMULSDrr",

        "DIVPSrm",
        "DIVPSrr",
        "DIVPDrm",
        "DIVPDrr",
        "VDIVPSrm",
        "VDIVPSrr",
        "VDIVPDrm",
        "VDIVPDrr",
        "VDIVSSrm",
        "VDIVSSrr",
        "VDIVSDrm",
        "VDIVSDrr",

        // integer SIMD
        "PADDBrm",
        "PADDBrr",
        "PADDWrm",
        "PADDWrr",
        "PADDDrm",
        "PADDDrr",
        "VPADDBrm",
        "VPADDBrr",
        "VPADDWrm",
        "VPADDWrr",
        "VPADDDrm",
        "VPADDDrr",

        "PSUBBrm",
        "PSUBBrr",
        "PSUBWrm",
        "PSUBWrr",
        "PSUBDrm",
        "PSUBDrr",
        "VPSUBBrm",
        "VPSUBBrr",
        "VPSUBWrm",
        "VPSUBWrr",
        "VPSUBDrm",
        "VPSUBDrr",

        "PMULLDrm",
        "PMULLDrr",
        "VPMULLDrm",
        "VPMULLDrr",

        // logical SIMD
        "ANDPSrm",
        "ANDPSrr",
        "ORPSrm",
        "ORPSrr",
        "XORPSrm",
        "XORPSrr",
        "VANDPSrm",
        "VANDPSrr",
        "VORPSrm",
        "VORPSrr",
        "VXORPSrm",
        "VXORPSrr",

        // min/max
        "MINPSrm",
        "MINPSrr",
        "MINPDrm",
        "MINPDrr",
        "MAXPSrm",
        "MAXPSrr",
        "MAXPDrm",
        "MAXPDrr",
        "VMINPSrm",
        "VMINPSrr",
        "VMINPDrm",
        "VMINPDrr",
        "VMAXPSrm",
        "VMAXPSrr",
        "VMAXPDrm",
        "VMAXPDrr",

        // conversion
        "CVTDQ2PSrm",
        "CVTDQ2PSrr",
        "CVTPS2DQrm",
        "CVTPS2DQrr",
        "VCVTDQ2PSrm",
        "VCVTDQ2PSrr",
        "VCVTPS2DQrm",
        "VCVTPS2DQrr",
        "VCVTDQ2PSrm",
        "VCVTDQ2PSrr",

        // shuffles
        "SHUFPSrm",
        "SHUFPSrr",
        "VSHUFPSrm",
        "VSHUFPSrr",

        // move instructions (packed)
        "MOVAPSrm",
        "MOVAPSrr",
        "MOVAPDrm",
        "MOVAPDrr",
        "MOVUPSrm",
        "MOVUPSrr",
        "MOVUPDrm",
        "MOVUPDrr",
        "VMOVAPSrm",
        "VMOVAPSrr",
        "VMOVAPDrm",
        "VMOVAPDrr",
        "VMOVUPSrm",
        "VMOVUPSrr",
        "VMOVUPDrm",
        "VMOVUPDrr",

        // fused multiply add
        "VFMADD132PSrm",
        "VFMADD213PSrm",
        "VFMADD231PSrm",
        "VFMADD132PDrr",
        "VFMADD213PDrr",
        "VFMADD231PDrr",
    };

    for (auto &MBB : MF) {
      for (auto MI = MBB.instr_begin(); MI != MBB.instr_end(); ++MI) {
        const MachineInstr &Instr = *MI;
        StringRef OpcodeName = TII->getName(Instr.getOpcode());

        bool isSimd = false;
        for (const auto &opcode : simd_opcodes) {
          if (OpcodeName == opcode) {
            isSimd = true;
            break;
          }
        }
        if (!isSimd)
          continue;

        DebugLoc DL = Instr.getDebugLoc();
        auto InsertPt = std::next(MI);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::MOV64rm), X86::RAX)
            .addReg(0)
            .addImm(1)
            .addReg(0)
            .addGlobalAddress(SimdCounter)
            .addImm(0);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::ADD64ri32), X86::RAX)
            .addReg(X86::RAX)
            .addImm(1);

        BuildMI(MBB, InsertPt, DL, TII->get(X86::MOV64mr))
            .addReg(0)
            .addImm(1)
            .addReg(0)
            .addGlobalAddress(SimdCounter)
            .addImm(0)
            .addReg(X86::RAX);

        Changed = true;
      }
    }
    return Changed;
  }
};

char InstrumentSimdPass::ID = 0;

} // namespace

static llvm::RegisterPass<InstrumentSimdPass>
    X("instrument-simd-x86", "Instrument SIMD instructions", false, false);
