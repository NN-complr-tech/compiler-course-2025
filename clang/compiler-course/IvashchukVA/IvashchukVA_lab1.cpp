#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;

namespace {

class EmptyAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<ASTConsumer>();
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

static FrontendPluginRegistry::Add<EmptyAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST", "Empty plugin");