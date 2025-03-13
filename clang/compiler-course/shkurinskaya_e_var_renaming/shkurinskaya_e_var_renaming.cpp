#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class PrefixVisitor final : public clang::RecursiveASTVisitor<PrefixVisitor> {
public:
  explicit PrefixVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter) {}

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getLocation().isInvalid() || !VD->getIdentifier())
      return true;

    std::string newName = VD->getNameAsString();
    if (VD->isStaticLocal()) {
      newName = "static_" + newName;
    } else if (VD->hasGlobalStorage() && !VD->isStaticLocal()) {
      newName = "global_" + newName;
    } else if (VD->isLocalVarDecl() && !VD->isStaticLocal()) {
      newName = "local_" + newName;
    }

    if (newName != VD->getNameAsString()) {
      m_rewriter.ReplaceText(VD->getLocation(), VD->getNameAsString().size(),
                             newName);
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *PVD) {
    if (PVD->getLocation().isInvalid() || !PVD->getIdentifier())
      return true;

    std::string newName = "param_" + PVD->getNameAsString();
    m_rewriter.ReplaceText(PVD->getLocation(), PVD->getNameAsString().size(),
	                   newName);
    return true;
  }

private:
  clang::Rewriter &m_rewriter;
};

class PrefixConsumer final : public clang::ASTConsumer {
public:
  explicit PrefixConsumer(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_visitor(context, rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  PrefixVisitor m_visitor;
};

class PrefixAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<PrefixConsumer>(&ci.getASTContext(), m_rewriter);
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

static clang::FrontendPluginRegistry::Add<PrefixAction>
    X("ClangAST_4", "Adding prefixes to variables");
