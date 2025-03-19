#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class UnusedVarsVisitor final
    : public clang::RecursiveASTVisitor<UnusedVarsVisitor> {
public:
  explicit UnusedVarsVisitor(clang::ASTContext *context,
                             clang::Rewriter &rewriter)
      : m_context(context), m_rewriter(rewriter) {}

  bool VisitParamVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed() && !param->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation location = param->getBeginLoc();
      if (clang::Rewriter::isRewritable(location)) {
        m_rewriter.InsertTextBefore(location, "[[maybe_unused]] ");
      } else {
        return false;
      }
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed() && !var->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation location = var->getBeginLoc();
      if (clang::Rewriter::isRewritable(location)) {
        m_rewriter.InsertTextBefore(location, "[[maybe_unused]] ");
      } else {
        return false;
      }
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &m_rewriter;
};

class UnusedVarsConsumer final : public clang::ASTConsumer {
public:
  explicit UnusedVarsConsumer(clang::ASTContext *context,
                              clang::Rewriter &rewriter)
      : m_visitor(context, rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  UnusedVarsVisitor m_visitor;
};

class UnusedVarsAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<UnusedVarsConsumer>(&ci.getASTContext(),
                                                m_rewriter);
  }

  void EndSourceFileAction() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "-h" || arg == "--help") {
        llvm::outs() << "The plugin adds the [[maybe_unused]] flag to "
                        "variables and parameters that are not in use.\n";
        return true;
      }
    }
    return true;
  }

private:
  clang::Rewriter m_rewriter;
};
} // namespace

static clang::FrontendPluginRegistry::Add<UnusedVarsAction>
    X("savchenko_m_UnusedVars_plugin",
      "Adding the [[maybe_unused]] flag to variables and parameters if they "
      "are not in use.");
