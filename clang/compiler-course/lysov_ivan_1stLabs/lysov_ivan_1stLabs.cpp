#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using llvm::outs;

namespace {

class UnusedVarLabel final : public clang::RecursiveASTVisitor<UnusedVarLabel> {
public:
  explicit UnusedVarLabel(clang::ASTContext *ctx, clang::Rewriter &rwrt)
      : context(ctx), rewriter(rwrt) {}

  bool VisitVarDecl(clang::VarDecl *var) {
      if (!var->isUsed() && !var->isImplicit() && !var->hasAttr<clang::UnusedAttr>()) {
          outs() << "Marking variable as maybe_unused: " << var->getNameAsString() << "\n";
          auto *attr = clang::UnusedAttr::CreateImplicit(*context);
          var->addAttr(attr);
          clang::SourceLocation loc = var->getSourceRange().getBegin();
          rewriter.InsertText(loc, "[[maybe_unused]] ", true, true);
      }
      return true;
    }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
      if (!param->isUsed() && !param->isImplicit() && !param->hasAttr<clang::UnusedAttr>()) {
          outs() << "Marking parameter as maybe_unused: " << param->getNameAsString() << "\n";
          auto *attr = clang::UnusedAttr::CreateImplicit(*context);
          param->addAttr(attr);
          clang::SourceLocation loc = param->getSourceRange().getBegin();
          rewriter.InsertText(loc, "[[maybe_unused]] ", true, true);
      }
      return true;
    }

private:
  clang::ASTContext *context;
  clang::Rewriter &rewriter;
};

class ASTProcessor final : public clang::ASTConsumer {
public:
  explicit ASTProcessor(clang::ASTContext *ctx, clang::Rewriter &rwrt)
      : Label(ctx, rwrt) {}

  void HandleTranslationUnit(clang::ASTContext &ctx) override {
      Label.TraverseDecl(ctx.getTranslationUnitDecl());
    }

private:
  UnusedVarLabel Label;
};

class PluginEntry final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
      rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
      return std::make_unique<ASTProcessor>(&ci.getASTContext(), rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci, const std::vector<std::string> &args) override {
      return true;
  }

  void EndSourceFileAction() override {
      rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

private:
  clang::Rewriter rewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<PluginEntry>
    X("unUsedVarPlugin_LysovIvan_FIIT3_ClangAST",
      "The plugin detects unused variables and function parameters in code and "
      "adds the [[maybe_unused]] attribute.");
