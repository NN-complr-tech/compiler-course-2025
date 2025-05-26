#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

namespace {
class PrefixVisitor : public clang::RecursiveASTVisitor<PrefixVisitor> {
public:
  explicit PrefixVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter) {}

  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var) {
      return true;
    }
    if (var->getName().empty()) {
      return true;
    }
    std::string prefix;
    if (var->isStaticLocal()) {
      prefix = "static_";
    } else if (var->isLocalVarDecl()) {
      prefix = "local_";
    } else if (var->hasGlobalStorage()) {
      prefix = "global_";
    }

    if (!prefix.empty()) {
      std::string newName = prefix + var->getName().str();
      renameVar(var, newName);
      m_renamedVars[var] = newName;
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (!param) {
      return true;
    }
    if (param->getName().empty()) {
      return true;
    }
    std::string newName = "param_" + param->getName().str();
    renameVar(param, newName);
    m_renamedVars[param] = newName;
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *expr) {
    if (!expr) {
      return true;
    }
    auto *decl = expr->getDecl();
    if (!decl) {
      return true;
    }
    if (decl->getName().empty()) {
      return true;
    }

    if (auto *var = clang::dyn_cast<clang::VarDecl>(decl)) {
      auto it = m_renamedVars.find(var);
      if (it != m_renamedVars.end()) {
        m_rewriter.ReplaceText(expr->getLocation(), var->getName().size(),
                               it->second);
      }
    }
    return true;
  }

private:
  clang::Rewriter &m_rewriter;
  std::unordered_map<clang::VarDecl *, std::string> m_renamedVars;

  void renameVar(clang::VarDecl *var, const std::string &newName) {
    m_rewriter.ReplaceText(var->getLocation(), var->getName().size(), newName);
  }
};

class PrefixConsumer : public clang::ASTConsumer {
public:
  explicit PrefixConsumer(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_visitor(context, rewriter), m_rewriter(rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_rewriter.getEditBuffer(context.getSourceManager().getMainFileID())
        .write(llvm::outs());
  }

private:
  PrefixVisitor m_visitor;
  clang::Rewriter &m_rewriter;
};

class PrefixAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    if (m_shouldExit) {
      return nullptr;
    }

    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<PrefixConsumer>(&ci.getASTContext(), m_rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "--help") {
        llvm::outs() << "PrefixesPlugin_RezantsevaAnastasia_FIIT1_ClangAST:\n";
        llvm::outs() << "This plugin adds appropriate prefixes to variables "
                        "and parameters in the code.\n";
        m_shouldExit = true;
        return true;
      }
    }
    return true;
  }

private:
  clang::Rewriter m_rewriter;
  bool m_shouldExit = false;
};
} // namespace

static clang::FrontendPluginRegistry::Add<PrefixAction>
    X("PrefixesPlugin_RezantsevaAnastasia_FIIT1_ClangAST",
      "Adds appropriate prefixes to objects and parameters");