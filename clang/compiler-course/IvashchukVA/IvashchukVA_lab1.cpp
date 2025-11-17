#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class AddMaybeUnusedVisitor : public RecursiveASTVisitor<AddMaybeUnusedVisitor> {
public:
  bool VisitVarDecl(VarDecl *VD) {
    if (!VD->hasInit() || VD->isImplicit() || VD->isFunctionOrMethodVarDecl()) {
      return true;
    }

    StringRef Name = VD->getName();
    if (Name.contains("unused")) {
      llvm::outs() << "[[maybe_unused]] ";
    }
    llvm::outs() << "int " << Name << " = ";
    if (auto *Init = VD->getInit()) {
      Init->printPretty(llvm::outs(), nullptr, PrintingPolicy(LangOptions()));
    }
    llvm::outs() << ";\n";
    return true;
  }
};

class AddMaybeUnusedConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Context) override {
    AddMaybeUnusedVisitor Visitor;
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class AddMaybeUnusedAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<AddMaybeUnusedConsumer>();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<AddMaybeUnusedAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST",
      "Adds [[maybe_unused]] to variables containing 'unused'");