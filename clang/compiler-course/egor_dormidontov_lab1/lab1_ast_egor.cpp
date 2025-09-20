#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <unordered_map>

using namespace clang;

namespace {

class PrefixCallbackVisitor
    : public RecursiveASTVisitor<PrefixCallbackVisitor> {
public:
  PrefixCallbackVisitor(ASTContext *context, Rewriter &rewriter)
      : Context(context), TheRewriter(rewriter) {}

  bool VisitParmVarDecl(ParmVarDecl *param) {
    const FunctionDecl *func = findParentFunction(param);
    if (!func)
      return true;
    std::string newName = "param_" + param->getNameAsString();
    funcDecl2Map[func][param] = newName;
    TheRewriter.ReplaceText(param->getLocation(),
                            param->getNameAsString().length(), newName);
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *func) {
    if (!func->hasBody())
      return true;
    funcDecl2Map[func] = std::unordered_map<const ValueDecl *, std::string>();
    return true;
  }

  bool VisitVarDecl(VarDecl *decl) {
    const FunctionDecl *func = findParentFunction(decl);
    if (func && decl->isLocalVarDecl() && !decl->isStaticLocal()) {
      std::string newName = "local_" + decl->getNameAsString();
      funcDecl2Map[func][decl] = newName;
      TheRewriter.ReplaceText(decl->getLocation(),
                              decl->getNameAsString().length(), newName);
      return true;
    }
    if (func && decl->isStaticLocal()) {
      std::string newName = "static_" + decl->getNameAsString();
      funcDecl2Map[func][decl] = newName;
      TheRewriter.ReplaceText(decl->getLocation(),
                              decl->getNameAsString().length(), newName);
      return true;
    }
    if (decl->isFileVarDecl() && decl->hasGlobalStorage() &&
        !decl->isStaticLocal()) {
      std::string newName = "global_" + decl->getNameAsString();
      TheRewriter.ReplaceText(decl->getLocation(),
                              decl->getNameAsString().length(), newName);
      return true;
    }
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *expr) {
    const ValueDecl *vd = dyn_cast<ValueDecl>(expr->getDecl());
    if (!vd)
      return true;
    if (!Context->getSourceManager().isWrittenInMainFile(expr->getLocation()))
      return true;
    const FunctionDecl *func = findParentFunction(expr);
    if (func) {
      auto fit = funcDecl2Map.find(func);
      if (fit != funcDecl2Map.end()) {
        auto it = fit->second.find(vd);
        if (it != fit->second.end()) {
          TheRewriter.ReplaceText(expr->getLocation(),
                                  vd->getNameAsString().length(), it->second);
        }
      }
    } else {
      if (const VarDecl *gvd = dyn_cast<VarDecl>(vd)) {
        if (gvd->hasGlobalStorage() && !gvd->isStaticLocal()) {
          std::string newName = "global_" + vd->getNameAsString();
          TheRewriter.ReplaceText(expr->getLocation(),
                                  vd->getNameAsString().length(), newName);
        }
      }
    }
    return true;
  }

private:
  ASTContext *Context;
  Rewriter &TheRewriter;
  std::unordered_map<const FunctionDecl *,
                     std::unordered_map<const ValueDecl *, std::string>>
      funcDecl2Map;

  template <typename T> const FunctionDecl *findParentFunction(const T *node) {
    const auto &parents = Context->getParents(*node);
    if (!parents.empty()) {
      if (const FunctionDecl *func = parents[0].template get<FunctionDecl>())
        return func;
      if (const Decl *parentDecl = parents[0].template get<Decl>())
        return findParentFunction(parentDecl);
      if (const Stmt *parentStmt = parents[0].template get<Stmt>())
        return findParentFunction(parentStmt);
    }
    return nullptr;
  }
};

class PrefixCallbackConsumer : public ASTConsumer {
public:
  PrefixCallbackConsumer(ASTContext *context, Rewriter &rewriter)
      : Visitor(context, rewriter) {}

  void HandleTranslationUnit(ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  PrefixCallbackVisitor Visitor;
};

class PrefixCallbackAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci,
                                                 llvm::StringRef) override {
    TheRewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<PrefixCallbackConsumer>(&ci.getASTContext(),
                                                    TheRewriter);
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  Rewriter TheRewriter;
};

} // namespace

static FrontendPluginRegistry::Add<PrefixCallbackAction>
    X("prefix_var_callback_plugin",
      "Add static_, local_, global_, param_ to variable names (unique logic)");