#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

namespace {
class PrefixVisitor final : public clang::RecursiveASTVisitor<PrefixVisitor> {
  clang::Rewriter &m_rewriter;
  std::unordered_map<clang::VarDecl *, std::string> rewrittenDecl;

public:
  explicit PrefixVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter) {}
  
  bool VisitVarDecl(clang::VarDecl *varDecl) {
    if (!varDecl) return true;

    std::string prevName = varDecl->getName().str();
    if (prevName.empty()) return true;

    std::string prefix;

    if (varDecl->isStaticLocal()) {
      prefix = "static_";
    } else if (varDecl->hasGlobalStorage()) {
      if (varDecl->getStorageClass() == clang::SC_Static) {
        prefix = "static_";
      } else if (varDecl->getStorageClass() == clang::SC_Extern) {
        prefix = "extern_";
      } else {
        prefix = "global_";
      }
    } else if (varDecl->isLocalVarDecl()) {
      prefix = "local_";
    }

    if (!prefix.empty()) {
      std::string newName = prefix + prevName;
      m_rewriter.ReplaceText(varDecl->getLocation(), prevName.size(), newName);
      rewrittenDecl[varDecl] = newName;
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *parDecl) {
    std::string prevName = parDecl->getName().str();
    if (prevName.empty()) return true;

    std::string newName = "param_" + prevName;
    m_rewriter.ReplaceText(parDecl->getLocation(), prevName.size(), newName);
    rewrittenDecl[parDecl] = newName;
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *declRef) {
    auto *varDecl = clang::dyn_cast<clang::VarDecl>(declRef->getDecl());
    if (!varDecl) return true;

    auto it = rewrittenDecl.find(varDecl);
    if (it != rewrittenDecl.end()) {
      std::string newName = it->second;
      std::string prevName = varDecl->getName().str();
      m_rewriter.ReplaceText(declRef->getLocation(), prevName.size(), newName);
    }
    return true;
  }
};

class PrefixConsumer final : public clang::ASTConsumer {
  clang::Rewriter &m_rewriter;
  PrefixVisitor m_visitor;

public:
  explicit PrefixConsumer(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter), m_visitor(context, rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_rewriter.getEditBuffer(context.getSourceManager().getMainFileID())
        .write(llvm::outs());
  }
};

class PrefixAction : public clang::PluginASTAction {
  clang::Rewriter m_rewriter;

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
};
} // namespace

static clang::FrontendPluginRegistry::Add<PrefixAction>
    X("PrefixPlugin_Khokhlov_andrey_FIIT2_ClangAST", "Some Description plugin");