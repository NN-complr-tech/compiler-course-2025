#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class UnusedVisitor final : public clang::RecursiveASTVisitor<UnusedVisitor> {
public:
  UnusedVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_context(context), m_rewriter(rewriter) {}
  bool VisitFuncParameterDecl(clang::ParmVarDecl *parameter) {
    if (!parameter->isUsed() && !parameter->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation Loc = parameter->getBeginLoc();
      if (clang::Rewriter::isRewritable(Loc)) {
        m_rewriter.InsertTextBefore(Loc, "[[maybe_unused]] ");
      } else
        return false;
    }
    return true;
  }
  bool VisitVarDecl(clang::VarDecl *variable) {
    if (!variable->isUsed() && !variable->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation Loc = variable->getBeginLoc();
      if (clang::Rewriter::isRewritable(Loc)) {
        m_rewriter.InsertTextBefore(Loc, "[[maybe_unused]] ");
      } else
        return false;
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  // Using rewriter - the main interface to rewrite buffers
  clang::Rewriter &m_rewriter;
};

class UnusedConsumer final : public clang::ASTConsumer {
public:
  UnusedConsumer(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_visitor(context, rewriter) {}
  void HandleTranslationUnit(clang::ASTContext &context) override {
    // TraverseDecl - Recursively visit a declaration
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  UnusedVisitor m_visitor;
};

class UnusedAction final : public clang::PluginASTAction {
private:
  clang::Rewriter m_rewriter;

public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<UnusedConsumer>(&ci.getASTContext(), m_rewriter);
  }

  void EndSourceFile() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (auto &i : args) {
      if (i == "--help") {
        llvm::outs()
            << "This plugin searches for unused variables or unused parameters "
               "of functions and marks them with attribute [[maybe__unused]]"
            << "\n";
      }
    }
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<UnusedAction>
    X("ClangAST_Kabalova_Valeria_FIIT1_ClangAST",
      "Adds attribute [[maybe_unused]] if the variable or parameter is unused");
