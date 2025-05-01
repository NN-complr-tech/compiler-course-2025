#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/X86/X86InstrInfo.h"
#include <map>

using namespace llvm;

namespace {

struct X86DecomposeAllFMAPass : public MachineFunctionPass {
  static char ID;
  X86DecomposeAllFMAPass() : MachineFunctionPass(ID) {
    initializeX86DecomposeAllFMAPassPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "X86 Decompose All FMA Instructions";
  }

  // Маппинг FMA opcode -> {MUL opcode, ADD opcode}
  const std::map<unsigned, std::pair<unsigned, unsigned>> FMAMap = {
      // Scalar Single-Precision (SS)
      {X86::VFMADD132SSrr, {X86::VMULSSrr, X86::VADDSSrr}},
      {X86::VFMADD213SSrr, {X86::VMULSSrr, X86::VADDSSrr}},
      {X86::VFMADD231SSrr, {X86::VMULSSrr, X86::VADDSSrr}},
      {X86::VFMADD132SSrm, {X86::VMULSSrm, X86::VADDSSrr}},
      {X86::VFMADD213SSrm, {X86::VMULSSrm, X86::VADDSSrr}},
      {X86::VFMADD231SSrm, {X86::VMULSSrm, X86::VADDSSrr}},

      // Packed Single-Precision (PS)
      {X86::VFMADD132PSrr, {X86::VMULPSrr, X86::VADDPSrr}},
      {X86::VFMADD213PSrr, {X86::VMULPSrr, X86::VADDPSrr}},
      {X86::VFMADD231PSrr, {X86::VMULPSrr, X86::VADDPSrr}},
      {X86::VFMADD132PSrm, {X86::VMULPSrm, X86::VADDPSrr}},
      {X86::VFMADD213PSrm, {X86::VMULPSrm, X86::VADDPSrr}},
      {X86::VFMADD231PSrm, {X86::VMULPSrm, X86::VADDPSrr}},

      // Scalar Double-Precision (SD)
      {X86::VFMADD132SDr, {X86::VMULSDrr, X86::VADDSDrr}},
      {X86::VFMADD213SDr, {X86::VMULSDrr, X86::VADDSDrr}},
      {X86::VFMADD231SDr, {X86::VMULSDrr, X86::VADDSDrr}},
      {X86::VFMADD132SDm, {X86::VMULSDrm, X86::VADDSDrr}},
      {X86::VFMADD213SDm, {X86::VMULSDrm, X86::VADDSDrr}},
      {X86::VFMADD231SDm, {X86::VMULSDrm, X86::VADDSDrr}},

      // Packed Double-Precision (PD)
      {X86::VFMADD132PDr, {X86::VMULPDrr, X86::VADDPDrr}},
      {X86::VFMADD213PDr, {X86::VMULPDrr, X86::VADDPDrr}},
      {X86::VFMADD231PDr, {X86::VMULPDrr, X86::VADDPDrr}},
      {X86::VFMADD132PDm, {X86::VMULPDrm, X86::VADDPDrr}},
      {X86::VFMADD213PDm, {X86::VMULPDrm, X86::VADDPDrr}},
      {X86::VFMADD231PDm, {X86::VMULPDrm, X86::VADDPDrr}},
  };

  bool processFMA(MachineInstr &MI, const TargetInstrInfo *TII,
                  MachineRegisterInfo &MRI) {
    unsigned Opc = MI.getOpcode();
    auto It = FMAMap.find(Opc);
    if (It == FMAMap.end())
      return false;

    unsigned MulOpc = It->second.first;
    unsigned AddOpc = It->second.second;

    // Операнды FMA: dst, src1, src2, src3
    Register Dest = MI.getOperand(0).getReg();
    MachineOperand &Src1 = MI.getOperand(1);
    MachineOperand &Src2 = MI.getOperand(2);
    MachineOperand &Src3 = MI.getOperand(3);

    // Создаём виртуальный регистр для результата умножения
    Register MulRes = MRI.createVirtualRegister(MRI.getRegClass(Dest));

    // Вставляем MUL инструкцию
    MachineInstrBuilder MulMI = BuildMI(*MI.getParent(), MI, MI.getDebugLoc(),
                                        TII->get(MulOpc), MulRes);

    // Добавляем операнды MUL
    MulMI.add(Src1).add(Src2);

    // Если MUL с памятью, добавляем адресные операнды
    if (MI.getDesc().mayLoad() && Src2.isMem())
      for (unsigned i = 3; i < MI.getNumOperands(); ++i)
        MulMI.add(MI.getOperand(i));

    // Вставляем ADD инструкцию
    MachineInstrBuilder AddMI =
        BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), TII->get(AddOpc), Dest);

    AddMI.addReg(MulRes).add(Src3);

    // Удаляем исходную FMA инструкцию
    MI.eraseFromParent();

    return true;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto MI = MBB.begin(), ME = MBB.end(); MI != ME;) {
        MachineInstr &Instr = *MI++;
        if (FMAMap.count(Instr.getOpcode())) {
          Changed |= processFMA(Instr, TII, MRI);
        }
      }
    }
    return Changed;
  }
};

char X86DecomposeAllFMAPass::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS(X86DecomposeAllFMAPass, "x86-decompose-all-fma",
                "X86 Decompose All FMA Instructions", false, false)

extern "C" LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeX86DecomposeAllFMAPassPass(PassRegistry &Registry) {
  initializeX86DecomposeAllFMAPassPass(Registry);
}

extern "C" LLVM_EXTERNAL_VISIBILITY llvm::FunctionPass *
createX86DecomposeAllFMAPass() {
  return new X86DecomposeAllFMAPass();
}
