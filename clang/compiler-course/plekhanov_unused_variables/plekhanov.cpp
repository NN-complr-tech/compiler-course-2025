#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class UnusedVariablesVisitor final
    : public clang::RecursiveASTVisitor<UnusedVariablesVisitor> {
public:
  UnusedVariablesVisitor(clang::ASTContext *Context, clang::Rewriter &R)
      : m_context(Context), TheRewriter(R) {}

  bool VisitParamVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed() && !param->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = param->getSourceRange().getBegin();
      TheRewriter.InsertText(loc, "[[maybe_unused]] ");
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed() && !var->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = var->getSourceRange().getBegin();
      TheRewriter.InsertText(loc, "[[maybe_unused]] ");
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &TheRewriter;
};

class UnusedVariablesConsumer final : public clang::ASTConsumer {
public:
  UnusedVariablesConsumer(clang::ASTContext *Context, clang::Rewriter &R)
      : Visitor(Context, R) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  UnusedVariablesVisitor Visitor;
};

class UnusedVariablesAction final : public clang::PluginASTAction {
private:
  clang::Rewriter TheRewriter;

public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<UnusedVariablesConsumer>(&CI.getASTContext(),
                                                     TheRewriter);
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<UnusedVariablesAction>
    X("FindUnused",
      "Find unused variables in code and mark them as the /MAYBE UNUSED/");
