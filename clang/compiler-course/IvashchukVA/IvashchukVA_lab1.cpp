#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;

namespace {

class AddMaybeUnusedAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    // Возвращаем nullptr - самый безопасный вариант
    return nullptr;
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