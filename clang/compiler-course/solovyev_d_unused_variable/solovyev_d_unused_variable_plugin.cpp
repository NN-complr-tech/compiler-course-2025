#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
namespace {
class UnusedVisitor final : public clang::RecursiveASTVisitor<UnusedVisitor> {
public:
  explicit UnusedVisitor(clang::ASTContext *context, clang::Rewriter &Rewriter)
      : m_context(context), MRewriter(Rewriter) {}
  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed() && !var->hasAttr<clang::UnusedAttr>()) {
      MRewriter.InsertText(var->getSourceRange().getBegin(),
                           "[[maybe_unused]] ", true, true);
      var->addAttr(clang::UnusedAttr::Create(*m_context));
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed() && !param->hasAttr<clang::UnusedAttr>()) {
      MRewriter.InsertText(param->getSourceRange().getBegin(),
                           "[[maybe_unused]] ", true, true);
      param->addAttr(clang::UnusedAttr::Create(*m_context));
    }
    return true;
  }

private:
  clang::Rewriter &MRewriter;
  clang::ASTContext *m_context;
};

class UnusedConsumer final : public clang::ASTConsumer {
public:
  explicit UnusedConsumer(clang::ASTContext *context, clang::Rewriter &Rewriter)
      : um_visitor(context, Rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    um_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  UnusedVisitor um_visitor;
};

class UnusedAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    MRewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<UnusedConsumer>(&ci.getASTContext(), MRewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "--help") {
        llvm::outs() << "Marking unused variables and function parameters as "
                        "[[maybe_unused]]\n";
        return true;
      }
    }
    return true;
  }

  void EndSourceFileAction() override {
    MRewriter.getEditBuffer(MRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter MRewriter;
};
} // namespace

static clang::FrontendPluginRegistry::Add<UnusedAction>
    X("unused_variable_plugin",
      "Marking unused variables and function parameters as [[maybe_unused]]");
