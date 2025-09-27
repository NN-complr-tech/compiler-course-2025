#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
namespace {
class MaybeUnusedVisitor : public RecursiveASTVisitor<MaybeUnusedVisitor> {
private:
  ASTContext &Context;
  Rewriter &rewriter;

public:
  explicit MaybeUnusedVisitor(ASTContext &context, Rewriter &RW)
      : Context(context), rewriter(RW) {}
  bool VisitVarDecl(VarDecl *VarDec) {
    if (VarDec->isUsed() || VarDec->hasAttr<clang::UnusedAttr>())
      return false;
    SourceLocation loc = VarDec->getBeginLoc();
    if (!Rewriter::isRewritable(loc))
      return false;
    rewriter.InsertTextBefore(loc, "[[maybe_unused]] ");
    return true;
  }
  bool VisitParmVarDecl(ParmVarDecl *ParDec) {
    if (ParDec->isUsed() || ParDec->hasAttr<clang::UnusedAttr>())
      return false;
    SourceLocation loc = ParDec->getBeginLoc();
    if (!Rewriter::isRewritable(loc))
      return false;
    rewriter.InsertTextBefore(loc, "[[maybe_unused]] ");
    return true;
  }
};

class MaybeUnusedASTConsumer : public ASTConsumer {
private:
  MaybeUnusedVisitor p_visitor;

public:
  explicit MaybeUnusedASTConsumer(ASTContext &context, Rewriter &R)
      : p_visitor(context, R) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    p_visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class MaybeUnusedAction : public PluginASTAction {
private:
  Rewriter rewriter;

public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MaybeUnusedASTConsumer>(CI.getASTContext(),
                                                    rewriter);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    SourceManager &SM = rewriter.getSourceMgr();
    rewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<MaybeUnusedAction>
    X("ClangAST_var2_Shpynov_Nikita_FIIT1_ClangAST",
      "Automatically inserts [[maybe_unused]] attribute on every potentially "
      "unused variable or parameter");
