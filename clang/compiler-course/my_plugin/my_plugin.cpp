#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>

using namespace clang;

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context, Rewriter &R)
      : Context(Context), Rewrite(R) {}

  bool VisitVarDecl(VarDecl *Decl) {
    if (!Decl)
      return false;

    std::string VarPrefix;
    if (Decl->isFileVarDecl() && !Decl->isStaticLocal()) {
      VarPrefix = "global_";
    } else if (Decl->isStaticLocal()) {
      VarPrefix = "static_";
    } else if (Decl->isLocalVarDecl() && !isa<ParmVarDecl>(Decl)) {
      VarPrefix = "local_";
    }

    if (!VarPrefix.empty()) {
      std::string NewID = VarPrefix + Decl->getNameAsString();
      SourceLocation StartLocation = Decl->getLocation();
      SourceLocation EndLocation =
          StartLocation.getLocWithOffset(Decl->getNameAsString().length());
      Rewrite.ReplaceText(SourceRange(StartLocation, EndLocation), NewID);

      FullSourceLoc FullLocation = Context->getFullLoc(StartLocation);
      if (FullLocation.isValid()) {
        llvm::outs() << "Found variable: " << Decl->getNameAsString() << " -> "
                     << NewID << " at "
                     << FullLocation.getSpellingLineNumber() << ":"
                     << FullLocation.getSpellingColumnNumber() << "\n";
      }
    }

    return true;
  }

  bool VisitParmVarDecl(ParmVarDecl *Decl) {
    if (!Decl)
      return false;

    std::string VarParmPrefix = "param_";
    std::string NewID = VarParmPrefix + Decl->getNameAsString();
    SourceLocation StartLocation = Decl->getLocation();
    SourceLocation EndLocation =
        StartLocation.getLocWithOffset(Decl->getNameAsString().length());
    Rewrite.ReplaceText(SourceRange(StartLocation, EndLocation), NewID);

    FullSourceLoc FullLocation = Context->getFullLoc(StartLocation);
    if (FullLocation.isValid()) {
      llvm::outs() << "Found parameter: " << Decl->getNameAsString() << " -> "
                   << NewID << " at " << FullLocation.getSpellingLineNumber()
                   << ":" << FullLocation.getSpellingColumnNumber() << "\n";
    }

    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *Expr) {
    if (!Expr)
      return false;

    VarDecl *Decl = dyn_cast<VarDecl>(Expr->getDecl());
    if (!Decl)
      return false;

    std::string VarPrefix;
    if (Decl->isFileVarDecl() && !Decl->isStaticLocal()) {
      VarPrefix = "global_";
    } else if (Decl->isStaticLocal()) {
      VarPrefix = "static_";
    } else if (Decl->isLocalVarDecl() && !isa<ParmVarDecl>(Decl)) {
      VarPrefix = "local_";
    } else if (isa<ParmVarDecl>(Decl)) {
      VarPrefix = "param_";
    }

    if (!VarPrefix.empty()) {
      std::string NewID = VarPrefix + Decl->getNameAsString();
      SourceLocation StartLocation = Expr->getLocation();
      SourceLocation EndLocation =
          StartLocation.getLocWithOffset(Decl->getNameAsString().length());
      Rewrite.ReplaceText(SourceRange(StartLocation, EndLocation), NewID);

      FullSourceLoc FullLocation = Context->getFullLoc(StartLocation);
      if (FullLocation.isValid()) {
        llvm::outs() << "Found reference: " << Decl->getNameAsString() << " -> "
                     << NewID << " at "
                     << FullLocation.getSpellingLineNumber() << ":"
                     << FullLocation.getSpellingColumnNumber() << "\n";
      }
    }

    return true;
  }

private:
  ASTContext *Context;
  Rewriter &Rewrite;
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
    X("ClangPlugin", "set prefixes");
//