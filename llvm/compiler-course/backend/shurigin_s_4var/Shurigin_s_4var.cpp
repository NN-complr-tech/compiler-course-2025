#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h" // <-- Добавлено для SmallVector
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "shurigin-fmsub-combine"

using namespace llvm;

namespace {

class SUBPass : public MachineFunctionPass {
public:
  static char ID;
  SUBPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "Simple Fused Multiply-Subtract Combine (Shurigin)";
  }

private:
  // Функции isSupportedMul, isSupportedSub, getFMAOpcode остаются без изменений
  // (из предыдущей версии)
  bool isSupportedMul(unsigned Opcode) const {
    switch (Opcode) {
    case X86::VMULSSrr:
    case X86::VMULSDrr:
    case X86::VMULPSrr:
    case X86::VMULPDrr:
      return true;
    default:
      return false;
    }
  }

  bool isSupportedSub(unsigned Opcode) const {
    switch (Opcode) {
    case X86::VSUBSSrr:
    case X86::VSUBSDrr:
    case X86::VSUBPSrr:
    case X86::VSUBPDrr:
      return true;
    default:
      return false;
    }
  }

  unsigned getFMAOpcode(unsigned SubOpcode) const {
    switch (SubOpcode) {
    case X86::VSUBSSrr:
      return X86::VFNMADD213SSr;
    case X86::VSUBSDrr:
      return X86::VFNMADD213SDr;
    case X86::VSUBPSrr:
      return X86::VFNMADD213PSr;
    case X86::VSUBPDrr:
      return X86::VFNMADD213PDr;
    default:
      return 0;
    }
  }
};

char SUBPass::ID = 0;

bool SUBPass::runOnMachineFunction(MachineFunction &MF) {

  const X86Subtarget &Subtarget = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = Subtarget.getInstrInfo();
  // const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo(); // Был
  // неиспользуемым, убираем

  LLVM_DEBUG(dbgs() << "\nRunning SUBPass on function: " << MF.getName()
                    << '\n');

  if (!Subtarget.hasFMA()) {
    LLVM_DEBUG(dbgs() << "  Target does not support FMA. Skipping pass.\n");
    return false;
  }

  bool Changed = false;
  // Вектор для хранения инструкций, которые нужно будет удалить
  SmallVector<MachineInstr *, 4> InstrsToDelete;

  for (MachineBasicBlock &MBB : MF) {
    LLVM_DEBUG(dbgs() << " Processing MBB: " << MBB.getName() << '\n');
    InstrsToDelete.clear(); // Очищаем для нового блока

    // Прямая итерация по инструкциям
    for (auto MBBI = MBB.begin(), MBBE = MBB.end(); MBBI != MBBE; ++MBBI) {
      MachineInstr &SubMI = *MBBI;

      LLVM_DEBUG(dbgs() << "  Checking instruction: "; SubMI.dump(););

      // 1. Поддерживаемая инструкция SUB?
      if (!isSupportedSub(SubMI.getOpcode())) {
        LLVM_DEBUG(dbgs() << "    Not a supported AVX SUB opcode.\n");
        continue;
      }

      // 2. Есть ли предыдущая инструкция?
      if (MBBI == MBB.begin()) {
        LLVM_DEBUG(
            dbgs() << "    SUB is the first instruction in the block.\n");
        continue;
      }
      // Получаем предыдущую инструкцию (итератор еще не сдвинут для нее)
      MachineInstr *MulMIPtr = &*std::prev(MBBI);
      if (!MulMIPtr)
        continue; // Маловероятно, но безопасно
      MachineInstr &MulMI = *MulMIPtr;

      LLVM_DEBUG(dbgs() << "    Preceding instruction: "; MulMI.dump(););

      // 3. Предыдущая инструкция - поддерживаемый MUL?
      if (!isSupportedMul(MulMI.getOpcode())) {
        LLVM_DEBUG(dbgs() << "    Preceding instruction is not a supported AVX "
                             "MUL opcode.\n");
        continue;
      }

      // 4. Проверка операндов
      // Упрощенно проверяем, что операнды 0, 1, 2 существуют и являются
      // регистрами
      if (MulMI.getNumExplicitOperands() < 3 || !MulMI.getOperand(0).isReg() ||
          !MulMI.getOperand(1).isReg() || !MulMI.getOperand(2).isReg()) {
        LLVM_DEBUG(
            dbgs()
            << "    MUL operands mismatch expected 3-register pattern.\n");
        continue;
      }
      if (SubMI.getNumExplicitOperands() < 3 || !SubMI.getOperand(0).isReg() ||
          !SubMI.getOperand(1).isReg() || !SubMI.getOperand(2).isReg()) {
        LLVM_DEBUG(
            dbgs()
            << "    SUB operands mismatch expected 3-register pattern.\n");
        continue;
      }

      Register MulDstReg = MulMI.getOperand(0).getReg();
      Register AReg = MulMI.getOperand(1).getReg();
      Register BReg = MulMI.getOperand(2).getReg();

      Register DstReg = SubMI.getOperand(0).getReg();
      Register CReg = SubMI.getOperand(1).getReg();
      Register TmpReg = SubMI.getOperand(2).getReg();

      // 5. Проверка зависимости
      if (MulDstReg != TmpReg) {
        // Используем LLVM_DEBUG для вывода регистров (требует
        // TargetRegisterInfo) Если TRI удален, используем простой вывод или
        // добавляем TRI назад
        LLVM_DEBUG(dbgs() << "    SUB's second operand register doesn't match "
                             "MUL's destination register.\n");
        // LLVM_DEBUG(dbgs() << "    SUB's second operand (Tmp:" <<
        // printReg(TmpReg, TRI)
        //                   << ") doesn't match MUL's destination (Tmp:" <<
        //                   printReg(MulDstReg, TRI) << ").\n");
        continue;
      }

      // 6. Проверка безопасности (isKill)
      if (!SubMI.getOperand(2).isKill()) {
        LLVM_DEBUG(dbgs() << "    Safety check failed: TmpReg is not killed by "
                             "SUB. Skipping optimization.\n");
        // LLVM_DEBUG(dbgs() << "    Safety check failed: TmpReg (" <<
        // printReg(TmpReg, TRI)
        //                   << ") is not killed by SUB. Skipping
        //                   optimization.\n");
        continue;
      }

      // 7. Получение FMA опкода
      unsigned FMAOpcode = getFMAOpcode(SubMI.getOpcode());
      if (FMAOpcode == 0) {
        LLVM_DEBUG(dbgs() << "    Could not find matching FMA opcode for SUB: "
                          << TII->getName(SubMI.getOpcode()) << "\n");
        continue;
      }

      LLVM_DEBUG(dbgs() << "  Pattern Found! Replacing with FMA.\n");
      LLVM_DEBUG(dbgs() << "    Using FMA Opcode: " << TII->getName(FMAOpcode)
                        << "\n");

      // 8. Создаем новую инструкцию FMA ПЕРЕД SubMI
      MachineInstr *NewMI =
          BuildMI(MBB, SubMI, SubMI.getDebugLoc(), TII->get(FMAOpcode), DstReg)
              .addReg(AReg, getRegState(MulMI.getOperand(1)))
              .addReg(BReg, getRegState(MulMI.getOperand(2)))
              .addReg(CReg, getRegState(SubMI.getOperand(1)))
              .getInstr();
      (void)NewMI;

      LLVM_DEBUG(dbgs() << "  New instruction created: "; NewMI->dump(););

      // 9. Добавляем старые инструкции в список на удаление
      InstrsToDelete.push_back(&MulMI);
      InstrsToDelete.push_back(&SubMI);

      Changed = true;
      LLVM_DEBUG(dbgs() << "  Marked instructions for deletion.\n");

      // ВАЖНО: При прямой итерации, после вставки и пометки на удаление,
      // лучше перейти к следующей инструкции, чтобы не проверять SubMI снова.
      // Итератор MBBI сам сдвинется на следующей итерации цикла for.
      // Если бы мы вставили что-то *после* SubMI, нужно было бы быть
      // осторожнее.
    }

    // Удаляем помеченные инструкции ПОСЛЕ завершения итерации по блоку
    if (!InstrsToDelete.empty()) {
      LLVM_DEBUG(dbgs() << " Deleting marked instructions for MBB "
                        << MBB.getName() << "\n");
      // Удаляем в обратном порядке добавления (сначала SubMI, потом MulMI)
      // Это может быть безопаснее, если между ними были связи
      for (MachineInstr *MI : reverse(InstrsToDelete)) {
        LLVM_DEBUG(dbgs() << "  Deleting: "; MI->dump(););
        MI->eraseFromParent();
      }
    }

  } // Конец цикла по блокам

  LLVM_DEBUG(dbgs() << "SUBPass finished. Changes made: " << Changed << "\n");
  return Changed;
}

} // namespace

static RegisterPass<SUBPass>
    X(DEBUG_TYPE, "Simple Fused Multiply-Subtract Combine (Shurigin)", false,
      false);
