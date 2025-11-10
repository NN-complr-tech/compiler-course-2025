#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

namespace {

class VariableRenamer final
    : public clang::RecursiveASTVisitor<VariableRenamer> {
public:
  explicit VariableRenamer(clang::Rewriter &Rewriter)
      : RewriterTool(Rewriter) {}

  bool VisitVarDecl(clang::VarDecl *Var) {
    if (shouldSkipVariable(Var))
      return true;

    std::string prefix = determineScopePrefix(Var);
    if (!prefix.empty()) {
      renameVariableDeclaration(Var, prefix);
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *Param) {
    if (shouldSkipVariable(Param))
      return true;

    renameParameter(Param);
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *Ref) {
    if (!Ref->getDecl() || Ref->getDecl()->getName().empty())
      return true;
    if (llvm::isa<clang::FunctionDecl>(Ref->getDecl()))
      return true;

    handleVariableReference(Ref);
    return true;
  }

private:
  bool shouldSkipVariable(clang::NamedDecl *Decl) const {
    return Decl->getName().empty();
  }

  std::string determineScopePrefix(const clang::VarDecl *Var) const {
    if (Var->isStaticLocal())
      return "static_";
    if (Var->isLocalVarDecl())
      return "local_";
    if (Var->hasGlobalStorage())
      return "global_";
    return "";
  }

  void renameVariableDeclaration(clang::VarDecl *Var,
                                 const std::string &Prefix) {
    std::string originalName = Var->getName().str();
    if (originalName.find(Prefix) == 0)
      return; // Already prefixed

    std::string newName = Prefix + originalName;
    variableNameMap[originalName] = newName;
    replaceVariableName(Var->getLocation(), originalName, newName);
  }

  void renameParameter(clang::ParmVarDecl *Param) {
    std::string originalName = Param->getName().str();
    std::string newName = "param_" + originalName;
    variableNameMap[originalName] = newName;
    replaceVariableName(Param->getLocation(), originalName, newName);
  }

  void handleVariableReference(clang::DeclRefExpr *Ref) {
    std::string originalName = Ref->getDecl()->getName().str();

    auto it = variableNameMap.find(originalName);
    if (it != variableNameMap.end()) {
      replaceVariableName(Ref->getLocation(), originalName, it->second);
    } else {
      processUncachedVariableReference(Ref, originalName);
    }
  }

  void processUncachedVariableReference(clang::DeclRefExpr *Ref,
                                        const std::string &OriginalName) {
    if (auto varDecl = llvm::dyn_cast<clang::VarDecl>(Ref->getDecl())) {
      varDecl = varDecl->getCanonicalDecl();
      std::string newName = generateNameForVariable(varDecl, OriginalName);

      if (!newName.empty()) {
        variableNameMap[OriginalName] = newName;
        replaceVariableName(Ref->getLocation(), OriginalName, newName);
      }
    }
  }

  std::string generateNameForVariable(clang::VarDecl *Var,
                                      const std::string &OriginalName) const {
    if (Var->hasGlobalStorage() && !Var->isStaticLocal()) {
      return "global_" + OriginalName;
    } else if (Var->isStaticLocal()) {
      return "static_" + OriginalName;
    } else if (llvm::isa<clang::ParmVarDecl>(Var)) {
      return "param_" + OriginalName;
    } else if (Var->isLocalVarDecl()) {
      return "local_" + OriginalName;
    }
    return "";
  }

  void replaceVariableName(clang::SourceLocation Location,
                           const std::string &OldName,
                           const std::string &NewName) {
    RewriterTool.ReplaceText(Location, OldName.length(), NewName);
  }

  clang::Rewriter &RewriterTool;
  std::unordered_map<std::string, std::string> variableNameMap;
};

class ASTProcessingConsumer final : public clang::ASTConsumer {
public:
  ASTProcessingConsumer(clang::ASTContext *Context, clang::Rewriter &Rewriter)
      : rewriter(Rewriter), visitor(Rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  clang::Rewriter &rewriter;
  VariableRenamer visitor;
};

class PrefixTransformationAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef) override {
    rewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
    return std::make_unique<ASTProcessingConsumer>(&Compiler.getASTContext(),
                                                   rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter rewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<PrefixTransformationAction>
    RegisterPlugin("variable_prefixer",
                   "Adds scope-based prefixes to variables");