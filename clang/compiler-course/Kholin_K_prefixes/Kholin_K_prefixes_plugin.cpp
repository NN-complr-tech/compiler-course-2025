#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <optional>
#include <set>

using namespace clang;

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context, Rewriter &R)
      : Context(Context), Rewrite(R) {}

  std::optional<std::string> getVarPrefix(const VarDecl *Decl) {
    if (Decl->isFileVarDecl() && !Decl->isStaticLocal()) {
      return "global_";
    } else if (Decl->isStaticLocal()) {
      return "static_";
    } else if (Decl->isLocalVarDecl() && !isa<ParmVarDecl>(Decl)) {
      return "local_";
    } else if (isa<ParmVarDecl>(Decl)) {
      return "param_";
    }
    return std::nullopt;
  }

  void renameVariable(Decl *Decl, SourceLocation StartLocation, const std::string &VarPrefix) {
    std::string NewID = VarPrefix + cast<VarDecl>(Decl)->getNameAsString();
    SourceLocation EndLocation =
        StartLocation.getLocWithOffset(cast<VarDecl>(Decl)->getNameAsString().length());
    Rewrite.ReplaceText(SourceRange(StartLocation, EndLocation), NewID);

    FullSourceLoc FullLocation = Context->getFullLoc(StartLocation);
    if (FullLocation.isValid()) {
      llvm::outs() << "Found variable: " << cast<VarDecl>(Decl)->getNameAsString() << " -> "
                   << NewID << " at " << FullLocation.getSpellingLineNumber() << ":"
                   << FullLocation.getSpellingColumnNumber() << "\n";
    }
  }

  bool VisitVarDecl(VarDecl *Decl) {
    if (!Decl)
      return false;

    std::optional<std::string> VarPrefix = getVarPrefix(Decl);
    if (VarPrefix.has_value()) {
      renameVariable(Decl, Decl->getLocation(), VarPrefix.value());
    }

    return true;
  }

  bool VisitParmVarDecl(ParmVarDecl *Decl) {
    if (!Decl)
      return false;

    std::optional<std::string> VarPrefix = getVarPrefix(Decl);
    if (VarPrefix.has_value()) {
      renameVariable(Decl, Decl->getLocation(), VarPrefix.value());
    }

    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *Expr) {
    if (!Expr)
      return false;

    VarDecl *Decl = dyn_cast<VarDecl>(Expr->getDecl());
    if (!Decl)
      return false;

    if (isa<ParmVarDecl>(Decl) && Flag.count(Decl) == 0) {
      std::optional<std::string> VarPrefix = getVarPrefix(Decl);
      if (VarPrefix.has_value()) {
        renameVariable(Decl, Expr->getLocation(), VarPrefix.value());
        Flag.insert(Decl);
      }
    } else if (!isa<ParmVarDecl>(Decl)) {
      std::optional<std::string> VarPrefix = getVarPrefix(Decl);
      if (VarPrefix.has_value()) {
        renameVariable(Decl, Expr->getLocation(), VarPrefix.value());
      }
    }

    return true;
  }

private:
  ASTContext *Context;
  Rewriter &Rewrite;
  std::set<const VarDecl *> Flag;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext *Context, Rewriter &R)
      : Visitor(Context, R) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(CompilerInstance &Compiler,
                    llvm::StringRef InFile) override {
    m_rewriter.setSourceMgr(Compiler.getSourceManager(),
                            Compiler.getLangOpts());
    return std::make_unique<FindNamedClassConsumer>(&Compiler.getASTContext(),
                                                    m_rewriter);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

private:
  Rewriter m_rewriter;
};

static clang::FrontendPluginRegistry::Add<FindNamedClassAction>
    X("PrefixesPlugin_KholinKirill_FIIT3_ClangAST", "set prefixes variables");
