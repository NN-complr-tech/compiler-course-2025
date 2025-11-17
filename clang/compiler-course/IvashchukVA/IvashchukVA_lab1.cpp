#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;

namespace {

class AddMaybeUnusedConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Context) override {}
};

class AddMaybeUnusedAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<AddMaybeUnusedConsumer>();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    for (const auto &Arg : Args) {
      if (Arg == "-help") {
        return false;
      }
    }
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return CmdlineBeforeMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<AddMaybeUnusedAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST",
      "Adds [[maybe_unused]] to variables containing 'unused'");