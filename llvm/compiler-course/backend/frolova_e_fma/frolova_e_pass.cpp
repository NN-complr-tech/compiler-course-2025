#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

using namespace llvm;

namespace {

using OperandOrder = std::array<unsigned, 3>;

class DecomposeFMAPass : public MachineFunctionPass {
public:
  static char ID;
  DecomposeFMAPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto MI = MBB.begin(); MI != MBB.end();) {
        MachineInstr &Instr = *MI++;
        if (isFMAOpcode(Instr.getOpcode())) {
          Changed |= decomposeFMA(Instr, MBB);
        }
      }
    }

    return Changed;
  }

private:
  const DenseMap<unsigned, OperandOrder> FMAOperandOrder = {
      {X86::VFMADD213SSr, {2, 1, 3}}, {X86::VFMADD213SDr, {2, 1, 3}},
      {X86::VFMADD213PSr, {2, 1, 3}}, {X86::VFMADD213PDr, {2, 1, 3}},
      {X86::VFMADD231SSr, {2, 3, 1}}, {X86::VFMADD231SDr, {2, 3, 1}},
      {X86::VFMADD231PSr, {2, 3, 1}}, {X86::VFMADD231PDr, {2, 3, 1}},
      {X86::VFMADD132SSr, {1, 3, 2}}, {X86::VFMADD132SDr, {1, 3, 2}},
      {X86::VFMADD132PSr, {1, 3, 2}}, {X86::VFMADD132PDr, {1, 3, 2}},
  };

  bool isFMAOpcode(unsigned Opcode) const {
    return FMAOperandOrder.contains(Opcode);
  }

  std::pair<unsigned, unsigned> getMulAddOpcodes(unsigned Opcode) const {
    if (Opcode == X86::VFMADD213SSr || Opcode == X86::VFMADD231SSr ||
        Opcode == X86::VFMADD132SSr)
      return {X86::VMULSSrr, X86::VADDSSrr};
    if (Opcode == X86::VFMADD213SDr || Opcode == X86::VFMADD231SDr ||
        Opcode == X86::VFMADD132SDr)
      return {X86::VMULSDrr, X86::VADDSDrr};
    if (Opcode == X86::VFMADD213PSr || Opcode == X86::VFMADD231PSr ||
        Opcode == X86::VFMADD132PSr)
      return {X86::VMULPSrr, X86::VADDPSrr};
    return {X86::VMULPDrr, X86::VADDPDrr};
  }

  bool decomposeFMA(MachineInstr &MI, MachineBasicBlock &MBB) {
    const X86InstrInfo *TII =
        MBB.getParent()->getSubtarget<X86Subtarget>().getInstrInfo();
    MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
    DebugLoc DL = MI.getDebugLoc();
    unsigned Opcode = MI.getOpcode();

    auto It = FMAOperandOrder.find(Opcode);
    assert(It != FMAOperandOrder.end() && "Unexpected FMA opcode!");
    OperandOrder Order = It->second;

    Register Dest = MI.getOperand(0).getReg();
    Register A = MI.getOperand(Order[0]).getReg();
    Register B = MI.getOperand(Order[1]).getReg();
    Register C = MI.getOperand(Order[2]).getReg();

    auto [MulOpcode, AddOpcode] = getMulAddOpcodes(Opcode);
    Register Temp = MRI.createVirtualRegister(MRI.getRegClass(A));

    BuildMI(MBB, MI, DL, TII->get(MulOpcode), Temp).addReg(A).addReg(B);
    BuildMI(MBB, MI, DL, TII->get(AddOpcode), Dest).addReg(C).addReg(Temp);

    MI.eraseFromParent();
    return true;
  }
};

char DecomposeFMAPass::ID = 0;

static RegisterPass<DecomposeFMAPass>
    X("fma-x86", "Decompose FMA into MUL + ADD", false, false);

} // namespace
