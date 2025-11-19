#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class AddMaybeUnusedVisitor : public RecursiveASTVisitor<AddMaybeUnusedVisitor> {
private
  Rewriter &rewriter;

public:
  explicit AddMaybeUnusedVisitor(Rewriter &RW) : rewriter(RW) {}

  bool VisitVarDecl(VarDecl *VD) {
    if (!VD->hasInit() || VD->isImplicit() || VD->isFunctionOrMethodVarDecl()) {
      return true;
    }

    StringRef Name = VD->getName();
    if (Name.contains("unused")) {
      SourceLocation loc = VD->getBeginLoc();
      if (loc.isValid()) {
        rewriter.InsertTextBefore(loc, "[[maybe_unused]] ");
      }
    }
    return true;
  }
};

class AddMaybeUnusedConsumer : public ASTConsumer {
private:
  AddMaybeUnusedVisitor visitor;

public:
  explicit AddMaybeUnusedConsumer(Rewriter &R) : visitor(R) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class AddMaybeUnusedAction : public PluginASTAction {
private:
  Rewriter rewriter;

public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<AddMaybeUnusedConsumer>(rewriter);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  void EndSourceFileAction() override {
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }
};

} // namespace

static FrontendPluginRegistry::Add<AddMaybeUnusedAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST",
      "Adds [[maybe_unused]] to variables containing 'unused'");