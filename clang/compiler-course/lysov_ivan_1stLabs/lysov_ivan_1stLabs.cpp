#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class UnusedVarMyVisitor final
    : public clang::RecursiveASTVisitor<UnusedVarMyVisitor> {
public:
  explicit UnusedVarMyVisitor(clang::ASTContext *ctx, clang::Rewriter &rwrt)
      : context(ctx), rewriter(rwrt) {}
  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed() && !var->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = var->getSourceRange().getBegin();
      rewriter.InsertText(loc, "[[maybe_unused]]");
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed() && !param->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = param->getSourceRange().getBegin();
      rewriter.InsertText(loc, "[[maybe_unused]]");
    }
    return true;
  }

private:
  clang::ASTContext *context;
  clang::Rewriter &rewriter;
};

class UnusedVarConsumer final : public clang::ASTConsumer {
public:
  explicit UnusedVarConsumer(clang::ASTContext *ctx, clang::Rewriter &rwrt)
      : Label(ctx, rwrt) {}

  void HandleTranslationUnit(clang::ASTContext &ctx) override {
    Label.TraverseDecl(ctx.getTranslationUnitDecl());
  }

private:
  UnusedVarMyVisitor Label;
};

class UnusedVarAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<UnusedVarConsumer>(&ci.getASTContext(), rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter rewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<UnusedVarAction>
    X("unUsedVarPlugin_LysovIvan_FIIT3_ClangAST",
      "The plugin detects unused variables and function parameters in code and "
      "adds the [[maybe_unused]] attribute.");
