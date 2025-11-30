#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class AddMaybeUnusedVisitor
    : public RecursiveASTVisitor<AddMaybeUnusedVisitor> {
private:
  Rewriter &TheRewriter;

public:
  explicit AddMaybeUnusedVisitor(Rewriter &RW) : TheRewriter(RW) {}

  bool visitVarDecl(VarDecl *VD) {
    if (!VD->hasInit() || VD->isImplicit() || VD->isFunctionOrMethodVarDecl()) {
      return true;
    }

    StringRef Name = VD->getName();
    if (Name.contains("unused")) {
      SourceLocation Loc = VD->getBeginLoc();
      if (Loc.isValid()) {
        TheRewriter.InsertTextBefore(Loc, "[[maybe_unused]] ");
      }
    }
    return true;
  }
};

class AddMaybeUnusedConsumer : public ASTConsumer {
private:
  AddMaybeUnusedVisitor TheVisitor;

public:
  explicit AddMaybeUnusedConsumer(Rewriter &R) : TheVisitor(R) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    TheVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class AddMaybeUnusedAction : public PluginASTAction {
private:
  Rewriter TheRewriter;

public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<AddMaybeUnusedConsumer>(TheRewriter);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }
};

} // namespace

static FrontendPluginRegistry::Add<AddMaybeUnusedAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST",
      "Adds [[maybe_unused]] to variables containing 'unused'");