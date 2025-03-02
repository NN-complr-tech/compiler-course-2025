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
  explicit ExampleVisitor(clang::ASTContext *Context, clang::Rewriter &Rewriter)
      : MRewriter(Rewriter) {}

  // Îáðàáîòêà îáúÿâëåíèé ïåðåìåííûõ
  bool VisitVarDecl(clang::VarDecl *Var) {
    if (Var->getName().empty()) {
      return true;
    }

    std::string Prefix;
    if (Var->isStaticLocal()) {
      Prefix = "static_";
    } else if (Var->isLocalVarDecl()) {
      Prefix = "local_";
    } else if (Var->hasGlobalStorage()) {
      Prefix = "global_";
    }

    if (!Prefix.empty()) {
      std::string OldName = Var->getName().str();
      std::string NewName = Prefix + OldName;
      MRenamedVars[OldName] = NewName;
      MRewriter.ReplaceText(Var->getLocation(), OldName.size(), NewName);
    }
    return true;
  }

  // Îáðàáîòêà ïàðàìåòðîâ ôóíêöèé
  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (param->getName().empty()) {
      return true;
    }

    std::string OldName = param->getName().str();
    std::string NewName = "param_" + OldName;
    MRenamedVars[OldName] = NewName;
    MRewriter.ReplaceText(param->getLocation(), OldName.size(), NewName);
    return true;
  }

  // Îáðàáîòêà èñïîëüçîâàíèÿ ïåðåìåííûõ
  bool VisitDeclRefExpr(clang::DeclRefExpr *Expr) {
    clang::ValueDecl *Decl = Expr->getDecl();
    if (Decl->getName().empty()) {
      return true;
    }

    std::string OldName = Decl->getName().str();

    auto It = MRenamedVars.find(OldName);
    if (It != MRenamedVars.end()) {
      std::string NewName = It->second;
      MRewriter.ReplaceText(Expr->getLocation(), OldName.size(), NewName);
    }
    return true;
  }

private:
  clang::Rewriter &MRewriter;
  std::unordered_map<std::string, std::string> MRenamedVars;
};

class ExampleConsumer final : public clang::ASTConsumer {
public:
  explicit ExampleConsumer(clang::ASTContext *Context,
                           clang::Rewriter &Rewriter)
      : MRewriter(Rewriter), MVisitor(Context, Rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    MVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  clang::Rewriter &MRewriter;
  ExampleVisitor MVisitor;
};

class ExampleAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Ci, llvm::StringRef) override {
    MRewriter.setSourceMgr(Ci.getSourceManager(), Ci.getLangOpts());
    return std::make_unique<ExampleConsumer>(&Ci.getASTContext(), MRewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &Ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    MRewriter.getEditBuffer(MRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter MRewriter;
};

} // namespace Shurigin_S_FI1_var4

static clang::FrontendPluginRegistry::Add<Shurigin_S_FI1_var4::ExampleAction>
    X("ClangAST_1_ShuriginS_FIIT1_ClangAST", "Adds prefixes to variables");