#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/LLVM.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;

namespace {

class AddMaybeUnusedConsumer : public ASTConsumer {
public:
  AddMaybeUnusedConsumer() = default;
  ~AddMaybeUnusedConsumer() override = default;
  
  void HandleTranslationUnit(ASTContext &Context) override {}
};

class AddMaybeUnusedAction : public PluginASTAction {
public:
  AddMaybeUnusedAction() = default;
  ~AddMaybeUnusedAction() override = default;
  
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<AddMaybeUnusedConsumer>();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return CmdlineBeforeMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<AddMaybeUnusedAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST", "Lab1 plugin");