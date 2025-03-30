#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class UnusedVarVisitor final
    : public clang::RecursiveASTVisitor<UnusedVarVisitor> {
public:
  UnusedVarVisitor(clang::ASTContext *Context, clang::Rewriter &R)
      : m_context(Context), TheRewriter(R) {}

  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed() && !var->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = var->getSourceRange().getBegin();
      TheRewriter.InsertText(loc, "[[maybe_unused]] ");
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed() && !param->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = param->getSourceRange().getBegin();
      TheRewriter.InsertText(loc, "[[maybe_unused]] ");
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &TheRewriter;
};

class UnusedVarConsumer final : public clang::ASTConsumer {
public:
  UnusedVarConsumer(clang::ASTContext *Context, clang::Rewriter &R)
      : Visitor(Context, R) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  UnusedVarVisitor Visitor;
};

class UnusedVarPlugin final : public clang::PluginASTAction {
private:
  clang::Rewriter TheRewriter;

public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<UnusedVarConsumer>(&CI.getASTContext(),
                                               TheRewriter);
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "--help" || arg == "-h") {
        llvm::outs()
            << "UnusedVarPlugin: Marks unused variables with [[maybe_unused]]\n"
            << "Usage: No arguments required\n"
            << "Options:\n"
            << "  --help, -h: Display this help message\n";
        return false;
      }
    }
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<UnusedVarPlugin>
    X("bessonov_e_unused_var_plugin",
      "Marks unused variables and parameters with [[maybe_unused]] attribute");