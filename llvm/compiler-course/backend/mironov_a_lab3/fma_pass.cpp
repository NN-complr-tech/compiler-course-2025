#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {

class FMAPass : public MachineFunctionPass {
private:
  bool is_fma(MachineInstr &instruction) {
    unsigned type = instruction.getOpcode();
    return instraction_table.find(type) != instraction_table.end();
  }

  bool op_is_reg(MachineInstr &instraction) {
    return !instraction.getOperand(0).isReg() ||
           !instraction.getOperand(1).isReg() ||
           !instraction.getOperand(2).isReg() ||
           !instraction.getOperand(3).isReg();
  }
  std::unordered_map<unsigned, std::pair<unsigned, unsigned>>
      instraction_table = {
          {llvm::X86::VFMADD132PSr, {llvm::X86::MULPSrr, llvm::X86::ADDPSrr}},
          {llvm::X86::VFMADD213PSr, {llvm::X86::MULPSrr, llvm::X86::ADDPSrr}},
          {llvm::X86::VFMADD231PSr, {llvm::X86::MULPSrr, llvm::X86::ADDPSrr}},
          {llvm::X86::VFMADD132PDr, {llvm::X86::MULPDrr, llvm::X86::ADDPDrr}},
          {llvm::X86::VFMADD213PDr, {llvm::X86::MULPDrr, llvm::X86::ADDPDrr}},
          {llvm::X86::VFMADD231PDr, {llvm::X86::MULPDrr, llvm::X86::ADDPDrr}},
          {llvm::X86::VFMADD132SSr, {llvm::X86::MULSSrr, llvm::X86::ADDSSrr}},
          {llvm::X86::VFMADD213SSr, {llvm::X86::MULSSrr, llvm::X86::ADDSSrr}},
          {llvm::X86::VFMADD231SSr, {llvm::X86::MULSSrr, llvm::X86::ADDSSrr}},
          {llvm::X86::VFMADD132SDr, {llvm::X86::MULSDrr, llvm::X86::ADDSDrr}},
          {llvm::X86::VFMADD213SDr, {llvm::X86::MULSDrr, llvm::X86::ADDSDrr}},
          {llvm::X86::VFMADD231SDr, {llvm::X86::MULSDrr, llvm::X86::ADDSDrr}},
          {llvm::X86::VFMADD132SSr_Int,
           {llvm::X86::MULSSrr_Int, llvm::X86::ADDSSrr_Int}},
          {llvm::X86::VFMADD213SSr_Int,
           {llvm::X86::MULSSrr_Int, llvm::X86::ADDSSrr_Int}},
          {llvm::X86::VFMADD231SSr_Int,
           {llvm::X86::MULSSrr_Int, llvm::X86::ADDSSrr_Int}},
          {llvm::X86::VFMADD132SDr_Int,
           {llvm::X86::MULSDrr_Int, llvm::X86::ADDSDrr_Int}},
          {llvm::X86::VFMADD213SDr_Int,
           {llvm::X86::MULSDrr_Int, llvm::X86::ADDSDrr_Int}},
          {llvm::X86::VFMADD231SDr_Int,
           {llvm::X86::MULSDrr_Int, llvm::X86::ADDSDrr_Int}},
      };
  std::unordered_map<unsigned, std::array<unsigned, 3>> fma_instruction_order =
      {{llvm::X86::VFMADD132PSr, {1, 3, 2}},
       {llvm::X86::VFMADD213PSr, {2, 1, 3}},
       {llvm::X86::VFMADD231PSr, {2, 3, 1}},
       {llvm::X86::VFMADD132PDr, {1, 3, 2}},
       {llvm::X86::VFMADD213PDr, {2, 1, 3}},
       {llvm::X86::VFMADD231PDr, {2, 3, 1}},
       {llvm::X86::VFMADD132SSr, {1, 3, 2}},
       {llvm::X86::VFMADD213SSr, {2, 1, 3}},
       {llvm::X86::VFMADD231SSr, {2, 3, 1}},
       {llvm::X86::VFMADD132SDr, {1, 3, 2}},
       {llvm::X86::VFMADD213SDr, {2, 1, 3}},
       {llvm::X86::VFMADD231SDr, {2, 3, 1}},
       {llvm::X86::VFMADD132SSr_Int, {1, 3, 2}},
       {llvm::X86::VFMADD213SSr_Int, {2, 1, 3}},
       {llvm::X86::VFMADD231SSr_Int, {2, 3, 1}},
       {llvm::X86::VFMADD132SDr_Int, {1, 3, 2}},
       {llvm::X86::VFMADD213SDr_Int, {2, 1, 3}},
       {llvm::X86::VFMADD231SDr_Int, {2, 3, 1}}};

public:
  static char ID;
  FMAPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool flag = false;

    const X86InstrInfo *inst_info =
        MF.getSubtarget<X86Subtarget>().getInstrInfo();
    MachineRegisterInfo &reg_info = MF.getRegInfo();
    std::vector<MachineInstr *> operations;

    for (MachineBasicBlock &block : MF) {
      operations.clear();
      for (MachineInstr &instraction : block) {

        if (!is_fma(instraction) || op_is_reg(instraction) ||
            instraction.getNumExplicitOperands() <= 3) {
          continue;
        }
        std::array<unsigned, 3> &order =
            fma_instruction_order[instraction.getOpcode()];

        std::pair<unsigned, unsigned> mul_add =
            instraction_table[instraction.getOpcode()];

        Register r0 = instraction.getOperand(0).getReg();
        Register r1 = instraction.getOperand(order[0]).getReg();
        Register r2 = instraction.getOperand(order[1]).getReg();
        Register r3 = instraction.getOperand(order[2]).getReg();

        const TargetRegisterClass *target =
            reg_info.getTargetRegisterInfo()->getRegClass(X86::FR32RegClassID);

        Register r4 = reg_info.createVirtualRegister(target);
        BuildMI(block, instraction, instraction.getDebugLoc(),
                inst_info->get(mul_add.first), r4)
            .addReg(r1)
            .addReg(r2);
        BuildMI(block, instraction, instraction.getDebugLoc(),
                inst_info->get(mul_add.second), r0)
            .addReg(r4)
            .addReg(r3);

        operations.push_back(&instraction);
        flag = true;
      }

      for (auto *ops : operations)
        ops->eraseFromParent();
    }

    return flag;
  }
};

char FMAPass::ID = 0;

} // namespace

static llvm::RegisterPass<FMAPass>
    X("fma_pass", "Replacing FMA instructions by combination of ADD and MUL",
      false, false);