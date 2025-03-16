#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <string>

namespace {

class PrefixVariableNames : public clang::RecursiveASTVisitor<PrefixVariableNames> {
public:
  explicit PrefixVariableNames(clang::Rewriter& rewriter)
      : rewriter_(rewriter) {}

  bool VisitVarDecl(clang::VarDecl* varDecl) {
    if (!varDecl->getName().empty()) {
      std::string prefix = determinePrefix(varDecl);
      if (!prefix.empty() && varDecl->getName().str().find(prefix) != 0) {
        std::string name = varDecl->getName().str();
        std::string mod_name = prefix + name;
        renamedVariables_[name] = mod_name;
        rewriter_.ReplaceText(varDecl->getLocation(), name.length(), mod_name);
      }
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl* paramVarDecl) {
    if (!paramVarDecl->getName().empty()) {
      std::string name = paramVarDecl->getName().str();
      std::string mod_name = "param_" + name;
      renamedVariables_[name] = mod_name;
      rewriter_.ReplaceText(paramVarDecl->getLocation(), name.length(), mod_name);
    }
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
    auto decl = expr->getDecl();
    if (decl) {
      std::string name = decl->getName().str();
      auto it = renamedVariables_.find(name);
      if (it != renamedVariables_.end()) {
        rewriter_.ReplaceText(expr->getLocation(), name.length(), it->second);
      }
    }
    return true;
  }

private:
  std::string determinePrefix(clang::VarDecl* varDecl) {
    if (varDecl->isStaticLocal()) {
      return "static_";
    } else if (varDecl->isLocalVarDecl()) {
      return "local_";
    } else if (varDecl->hasGlobalStorage()) {
      return "global_";
    }
    return "";
  }

  clang::Rewriter& rewriter_;
  std::unordered_map<std::string, std::string> renamedVariables_;
};

class PrefixConsumer : public clang::ASTConsumer {
public:
  explicit PrefixConsumer(clang::Rewriter& rewriter) : visitor_(rewriter) {}

  void HandleTranslationUnit(clang::ASTContext& context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  PrefixVariableNames visitor_;
};

class PrefixAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& ci, llvm::StringRef) override {
    rewriter_.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<PrefixConsumer>(rewriter_);
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    rewriter_.getEditBuffer(rewriter_.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter rewriter_;
};

} // namespace

static clang::FrontendPluginRegistry::Add<PrefixAction>
X("ClangAST_1_Solovev_a_FIIT1_ClangAST", "Prefixes variables based on type");
