//===- IvashchukVA_lab4.cpp - MLIR Call Counter Pass ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

using namespace mlir;

namespace {
struct CallCounterPass
    : public PassWrapper<CallCounterPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CallCounterPass)

  StringRef getArgument() const final { return "call-counter"; }

  StringRef getDescription() const final {
    return "Count function calls and add call_count attribute";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();

    for (auto func : module.getOps<func::FuncOp>()) {
      int callCount = 0;

      func.walk([&](Operation *op) {
        if (isa<func::CallOp>(op)) {
          callCount++;
        }
      });

      if (callCount > 0) {
        func->setAttr("call_count",
                      IntegerAttr::get(IntegerType::get(module.getContext(), 64),
                                       callCount));
      }
    }
  }
};
} // namespace

void registerCallCounterPass() {
  PassRegistration<CallCounterPass>();
}