#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

using namespace llvm;

namespace {

struct FMADecompositionPass : public MachineFunctionPass {
public:
  static char ID;
  FMADecompositionPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool Modified = false;

    for (auto &Block : MF) {
      for (auto InstrIter = Block.begin(); InstrIter != Block.end();) {
        MachineInstr &CurrentInstr = *InstrIter++;
        if (isFMAInstruction(CurrentInstr.getOpcode())) {
          Modified |= transformFMA(CurrentInstr, Block);
        }
      }
    }

    return Modified;
  }

private:
  struct FMAConfig {
    unsigned op1_idx;
    unsigned op2_idx; 
    unsigned op3_idx;
  };

  const DenseMap<unsigned, FMAConfig> FMAConfigs = {
    {X86::VFMADD213SSr, {2, 1, 3}}, {X86::VFMADD213SDr, {2, 1, 3}},
    {X86::VFMADD213PSr, {2, 1, 3}}, {X86::VFMADD213PDr, {2, 1, 3}},
    {X86::VFMADD231SSr, {2, 3, 1}}, {X86::VFMADD231SDr, {2, 3, 1}},
    {X86::VFMADD231PSr, {2, 3, 1}}, {X86::VFMADD231PDr, {2, 3, 1}},
    {X86::VFMADD132SSr, {1, 3, 2}}, {X86::VFMADD132SDr, {1, 3, 2}},
    {X86::VFMADD132PSr, {1, 3, 2}}, {X86::VFMADD132PDr, {1, 3, 2}},
  };

  bool isFMAInstruction(unsigned Op) const {
    return FMAConfigs.count(Op) > 0;
  }

  struct OpcodePair {
    unsigned mul_op;
    unsigned add_op;
  };

  OpcodePair getReplacementOpcodes(unsigned OriginalOp) const {
    switch (OriginalOp) {
    case X86::VFMADD213SSr: case X86::VFMADD231SSr: case X86::VFMADD132SSr:
      return {X86::VMULSSrr, X86::VADDSSrr};
    case X86::VFMADD213SDr: case X86::VFMADD231SDr: case X86::VFMADD132SDr:
      return {X86::VMULSDrr, X86::VADDSDrr};
    case X86::VFMADD213PSr: case X86::VFMADD231PSr: case X86::VFMADD132PSr:
      return {X86::VMULPSrr, X86::VADDPSrr};
    default:
      return {X86::VMULPDrr, X86::VADDPDrr};
    }
  }

  bool transformFMA(MachineInstr &FMAInstr, MachineBasicBlock &ParentBlock) {
    const X86InstrInfo *InstrInfo = 
        ParentBlock.getParent()->getSubtarget<X86Subtarget>().getInstrInfo();
    MachineRegisterInfo &RegInfo = ParentBlock.getParent()->getRegInfo();
    DebugLoc Location = FMAInstr.getDebugLoc();
    unsigned OriginalOpcode = FMAInstr.getOpcode();

    auto Config = FMAConfigs.find(OriginalOpcode);
    assert(Config != FMAConfigs.end() && "Invalid FMA opcode!");
    
    Register ResultReg = FMAInstr.getOperand(0).getReg();
    Register Op1 = FMAInstr.getOperand(Config->second.op1_idx).getReg();
    Register Op2 = FMAInstr.getOperand(Config->second.op2_idx).getReg();
    Register Op3 = FMAInstr.getOperand(Config->second.op3_idx).getReg();

    auto [MulOp, AddOp] = getReplacementOpcodes(OriginalOpcode);
    Register IntermediateReg = RegInfo.createVirtualRegister(RegInfo.getRegClass(Op1));

    // Create multiplication instruction
    BuildMI(ParentBlock, FMAInstr, Location, InstrInfo->get(MulOp), IntermediateReg)
        .addReg(Op1).addReg(Op2);
    
    // Create addition instruction  
    BuildMI(ParentBlock, FMAInstr, Location, InstrInfo->get(AddOp), ResultReg)
        .addReg(Op3).addReg(IntermediateReg);

    FMAInstr.removeFromParent();
    return true;
  }
};

char FMADecompositionPass::ID = 0;

static RegisterPass<FMADecompositionPass>
    RegisterPass("fma-decomposition", "FMA instruction decomposition", false, false);

} // namespace