#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

class AddMaybeUnusedVisitor
    : public RecursiveASTVisitor<AddMaybeUnusedVisitor> {
private:
  Rewriter &TheRewriter;

public:
  explicit AddMaybeUnusedVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitVarDecl(VarDecl *VD) {
    if (!VD->hasInit() || VD->isImplicit() || VD->isFunctionOrMethodVarDecl()) {
      return true;
    }

    StringRef Name = VD->getName();
    if (Name.contains("unused")) {
      SourceLocation Loc = VD->getBeginLoc();
      if (Loc.isValid()) {
        TheRewriter.InsertText(Loc, "[[maybe_unused]] ");
      }
    }

    return true;
  }
};

class AddMaybeUnusedConsumer : public ASTConsumer {
private:
  Rewriter TheRewriter;
  AddMaybeUnusedVisitor Visitor;

public:
  explicit AddMaybeUnusedConsumer(CompilerInstance &CI)
      : TheRewriter(CI.getSourceManager(), CI.getLangOpts()),
        Visitor(TheRewriter) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    TheRewriter.getEditBuffer