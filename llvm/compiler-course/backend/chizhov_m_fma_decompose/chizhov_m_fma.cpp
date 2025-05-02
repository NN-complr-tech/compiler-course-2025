#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "X86RegisterInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

namespace {

class FMADecomposePass : public llvm::MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : llvm::MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override {
    const auto &ST = MF.getSubtarget<llvm::X86Subtarget>();
    if (!ST.hasFMA())
      return false;

    const auto *TII = ST.getInstrInfo();
    auto &MRI = MF.getRegInfo();
    bool changed = false;

    // Map FMA opcodes to corresponding MUL/ADD instructions
    static const std::unordered_map<unsigned, std::pair<unsigned, unsigned>>
        FMAtoMULADD = {
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
            {llvm::X86::VFMADD132PSYr, {llvm::X86::VMULPSrr, llvm::X86::VADDPSrr}},
            {llvm::X86::VFMADD213PSYr, {llvm::X86::VMULPSrr, llvm::X86::VADDPSrr}},
            {llvm::X86::VFMADD231PSYr, {llvm::X86::VMULPSrr, llvm::X86::VADDPSrr}},
            {llvm::X86::VFMADD132PDYr, {llvm::X86::VMULPDrr, llvm::X86::VADDPDrr}},
            {llvm::X86::VFMADD213PDYr, {llvm::X86::VMULPDrr, llvm::X86::VADDPDrr}},
            {llvm::X86::VFMADD231PDYr, {llvm::X86::VMULPDrr, llvm::X86::VADDPDrr}},
    };

    for (auto &MBB : MF) {
      llvm::SmallVector<llvm::MachineInstr *, 8> toRemove;

      for (auto &MI : MBB) {
        auto It = FMAtoMULADD.find(MI.getOpcode());
        if (It == FMAtoMULADD.end())
          continue;

        auto [MulOpc, AddOpc] = It->second;
        auto &DL = MI.getDebugLoc();

        // Get operands: res, a, b, c
        auto res = MI.getOperand(0).getReg();
        auto var1 = MI.getOperand(1).getReg();
        auto var2 = MI.getOperand(2).getReg();
        auto var3 = MI.getOperand(3).getReg();

        const llvm::TargetRegisterClass *RC = MRI.getRegClass(var1);

        // Create temporary register
        auto Tmp = MRI.createVirtualRegister(RC);

        // Insert new instructions
        BuildMI(MBB, MI, DL, TII->get(MulOpc), Tmp).addReg(var1).addReg(var2);
        BuildMI(MBB, MI, DL, TII->get(AddOpc), res).addReg(var3).addReg(Tmp);

        toRemove.push_back(&MI);
        changed = true;
      }

      // Remove original FMA instructions
      for (auto *MI : toRemove)
        MI->eraseFromParent();
    }

    return changed;
  }
};

char FMADecomposePass::ID = 0;

} // namespace


static llvm::RegisterPass<FMADecomposePass>
    X("fma-decompose",
      "Decomposes FMA instructions into MUL and ADD", false, false);