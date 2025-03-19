#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

namespace {
class PrefixVarVisitor : public clang::RecursiveASTVisitor<PrefixVarVisitor> {
public:
  PrefixVarVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter) {}

  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var || var->getName().empty()) {
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
      std::string newName = prefix + var->getNameAsString();
      renameVar(var, newName);
      m_renamedVars[var] = newName;
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (!param || param->getName().empty()) {
      return true;
    }
    std::string newName = "param_" + param->getNameAsString();
    renameVar(param, newName);
    m_renamedVars[param] = newName;
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *expr) {
    auto *decl = expr->getDecl();
    if (!expr || !decl || decl->getName().empty()) {
      return true;
    }
    if (auto *var = clang::dyn_cast<clang::VarDecl>(decl)) {
      auto it = m_renamedVars.find(var);
      if (it != m_renamedVars.end()) {
        m_rewriter.ReplaceText(expr->getLocation(),
                               var->getNameAsString().size(), it->second);
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

class PrefixVarConsumer : public clang::ASTConsumer {
public:
  PrefixVarConsumer(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_visitor(context, rewriter), m_rewriter(rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_rewriter.getEditBuffer(context.getSourceManager().getMainFileID())
        .write(llvm::outs());
  }

private:
  PrefixVarVisitor m_visitor;
  clang::Rewriter &m_rewriter;
};

class PrefixVarAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    if (m_shouldExit) {
      return nullptr;
    }

    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<PrefixVarConsumer>(&ci.getASTContext(), m_rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "--help") {
        llvm::outs() << "PrefixVarPlugin_by_Korobeinikov_Arseny:\n"
                     << "This plugin changes names by adding prefixes to "
                        "variables and parameters.\n";
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

static clang::FrontendPluginRegistry::Add<PrefixVarAction>
    X("PrefixVarPlugin_Korobeinikov_FIIT1_ClangAST",
      "Changes names by adding prefixes to variables and parameters");