#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class ExampleVisitor : public clang::RecursiveASTVisitor<ExampleVisitor> {
public:
  explicit ExampleVisitor(clang::ASTContext *context, clang::Rewriter &rewriter)
      : m_rewriter(rewriter) {}

  bool VisitVarDecl(clang::VarDecl *var) {
    if (var->isLocalVarDecl()) {
      if (var->isStaticLocal()) {
        renameVar(var, "static_");
      } else {
        renameVar(var, "local_");
      }
    } else if (var->isFileVarDecl()) {
      if (var->getStorageClass() == clang::SC_Static) {
        renameVar(var, "static_global_");
      } else {
        renameVar(var, "global_");
      }
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    llvm::StringRef oldName = param->getName();
    if (oldName.empty())
      return true;

    std::string newName = "param_" + oldName.str();
    std::string defaultValueStr;
    if (param->hasDefaultArg()) {
      clang::Expr *defaultArg = param->getDefaultArg();
      llvm::raw_string_ostream stream(defaultValueStr);
      defaultArg->printPretty(stream, nullptr,
                              clang::PrintingPolicy(clang::LangOptions()));
      stream.flush();
      newName += "_default_" + defaultValueStr;
      clang::SourceLocation loc = param->getLocation();
      if (loc.isValid() && m_rewriter.getSourceMgr().isWrittenInMainFile(loc)) {
        m_rewriter.ReplaceText(loc, oldName.size() + defaultValueStr.size() + 3,
                               newName);
      }
    } else {
      clang::SourceLocation loc = param->getLocation();
      if (loc.isValid() && m_rewriter.getSourceMgr().isWrittenInMainFile(loc)) {
        m_rewriter.ReplaceText(loc, oldName.size(), newName);
      }
    }
    renamedVars[param] = newName;
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *expr) {
    if (auto *var = llvm::dyn_cast<clang::VarDecl>(expr->getDecl())) {
      llvm::StringRef oldName = var->getName();

      if (!shouldRename(var))
        return true;

      clang::SourceLocation loc = expr->getLocation();
      if (loc.isValid() && m_rewriter.getSourceMgr().isWrittenInMainFile(loc)) {
        std::string newName = getPrefixedName(var);
        m_rewriter.ReplaceText(loc, oldName.size(), newName);
      }
    }
    return true;
  }

private:
  clang::Rewriter &m_rewriter;
  llvm::DenseMap<clang::VarDecl *, std::string> renamedVars;

  bool shouldRename(clang::VarDecl *var) { return renamedVars.count(var) > 0; }

  std::string getPrefixedName(clang::VarDecl *var) { return renamedVars[var]; }

  void renameVar(clang::VarDecl *var, llvm::StringRef prefix) {
    llvm::StringRef oldName = var->getName();
    if (oldName.empty())
      return;

    std::string newName = (prefix + oldName).str();

    clang::SourceLocation loc = var->getLocation();
    if (loc.isValid() && m_rewriter.getSourceMgr().isWrittenInMainFile(loc)) {
      m_rewriter.ReplaceText(loc, oldName.size(), newName);
    }

    renamedVars[var] = newName;
  }
};

class ExampleConsumer : public clang::ASTConsumer {
public:
  explicit ExampleConsumer(clang::ASTContext *context,
                           clang::Rewriter &rewriter)
      : m_visitor(context, rewriter), m_rewriter(rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());

    auto &buffer =
        m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID());
    if (buffer.size() > 0) {
      buffer.write(llvm::outs());
    }
  }

private:
  ExampleVisitor m_visitor;
  clang::Rewriter &m_rewriter;
};

class ExampleAction : public clang::PluginASTAction {
public:
  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<ExampleConsumer>(&ci.getASTContext(), m_rewriter);
  }

private:
  clang::Rewriter m_rewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<ExampleAction>
    X("FirstPlugin_KozlovaEkaterina_FIIT3_ClangAST",
      "Adds prefixes to variables");
