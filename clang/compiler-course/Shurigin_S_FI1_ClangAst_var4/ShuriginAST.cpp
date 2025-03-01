#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

namespace Shurigin_S_FI1_var4 {

class ExampleVisitor final : public clang::RecursiveASTVisitor<ExampleVisitor> {
public:
  explicit ExampleVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter) {}

  // ќбработка объ€влений переменных
  bool VisitVarDecl(clang::VarDecl *var) {
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
      std::string oldName = var->getName().str();
      std::string newName = prefix + oldName;
      m_renamedVars[oldName] = newName;
      m_rewriter.ReplaceText(var->getLocation(), oldName.size(), newName);
    }
    return true;
  }

  // ќбработка параметров функций
  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (param->getName().empty()) {
      return true;
    }

    std::string oldName = param->getName().str();
    std::string newName = "param_" + oldName;
    m_renamedVars[oldName] = newName;
    m_rewriter.ReplaceText(param->getLocation(), oldName.size(), newName);
    return true;
  }

  // ќбработка использовани€ переменных
  bool VisitDeclRefExpr(clang::DeclRefExpr *expr) {
    clang::ValueDecl *decl = expr->getDecl();
    if (decl->getName().empty()) {
      return true;
    }

    std::string oldName = decl->getName().str();

    auto it = m_renamedVars.find(oldName);
    if (it != m_renamedVars.end()) {
      std::string newName = it->second;
      m_rewriter.ReplaceText(expr->getLocation(), oldName.size(), newName);
    }
    return true;
  }

private:
  clang::Rewriter &m_rewriter;
  std::unordered_map<std::string, std::string> m_renamedVars;
};

class ExampleConsumer final : public clang::ASTConsumer {
public:
  explicit ExampleConsumer(clang::ASTContext *context,
                           clang::Rewriter &rewriter)
      : m_rewriter(rewriter), m_visitor(context, rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  clang::Rewriter &m_rewriter;
  ExampleVisitor m_visitor;
};

class ExampleAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<ExampleConsumer>(&ci.getASTContext(), m_rewriter);
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

} // namespace Shurigin_S_FI1_var4

static clang::FrontendPluginRegistry::Add<Shurigin_S_FI1_var4::ExampleAction>
    X("ClangAST_1_ShuriginS_FIIT1_ClangAST", "Adds prefixes to variables");
