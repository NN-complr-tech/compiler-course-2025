#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class myVisitor final : public clang::RecursiveASTVisitor<myVisitor> {
public:
  explicit myVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_context(context), m_rewriter(rewriter) {}

  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed()) {
      clang::SourceLocation loc = var->getLocation();
      if (loc.isValid()) {
        m_rewriter.InsertText(loc, "[[maybe_unused]] ");
      } else
        return false;
    }
    return true;
  }
  bool VisitParamVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed()) {
      clang::SourceLocation loc = param->getLocation();
      if (loc.isValid()) {
        m_rewriter.InsertText(loc, "[[maybe_unused]] ");
      } else
        return false;
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &m_rewriter;
};

class myConsumer final : public clang::ASTConsumer {
public:
  explicit myConsumer(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_visitor(context, rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  myVisitor m_visitor;
};

class myAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<myConsumer>(&ci.getASTContext(), m_rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter m_rewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<myAction>
    X("UserDataTypePlugin_beresnev_a_clang",
      "Mark unused variables and parameters with [[maybe_unused]]");