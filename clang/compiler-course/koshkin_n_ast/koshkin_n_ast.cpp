#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class MaybeUnusedVisitor final
    : public RecursiveASTVisitor<MaybeUnusedVisitor> {
public:
  explicit MaybeUnusedVisitor(ASTContext *Context, Rewriter &R)
      : Context(Context), TheRewriter(R) {}

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->hasBody() || FD->isImplicit())
      return true;

    for (unsigned I = 0, N = FD->getNumParams(); I < N; ++I) {
      ParmVarDecl *PVD = FD->getParamDecl(I);
      if (PVD && PVD->getName() == "unused" && !PVD->hasAttr<UnusedAttr>()) {
        SourceLocation Loc = PVD->getBeginLoc();
        if (Loc.isValid())
          TheRewriter.InsertText(Loc, "[[maybe_unused]] ",
                                 /*InsertBefore=*/true,
                                 /*IndentNewLines=*/true);
      }
    }

    Stmt *Body = FD->getBody();
    if (Body)
      ProcessLocalVars(Body);

    return true;
  }

private:
  void ProcessLocalVars(Stmt *S) {
    if (!S)
      return;

    if (auto *DS = dyn_cast<DeclStmt>(S)) {
      for (auto *D : DS->decls()) {
        if (auto *VD = dyn_cast<VarDecl>(D)) {
          if (VD->isLocalVarDecl() && !VD->isImplicit() &&
              VD->getName() == "unused" && !VD->hasAttr<UnusedAttr>()) {
            SourceLocation Loc = VD->getBeginLoc();
            if (Loc.isValid())
              TheRewriter.InsertText(Loc, "[[maybe_unused]] ",
                                     /*InsertBefore=*/true,
                                     /*IndentNewLines=*/false);
          }
        }
      }
    }

    for (Stmt *Child : S->children())
      ProcessLocalVars(Child);
  }

  ASTContext *Context;
  Rewriter &TheRewriter;
};

class MaybeUnusedConsumer final : public ASTConsumer {
public:
  explicit MaybeUnusedConsumer(ASTContext *Context, Rewriter &R)
      : Visitor(Context, R) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  MaybeUnusedVisitor Visitor;
};

class MaybeUnusedAction final : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI,
                    llvm::StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MaybeUnusedConsumer>(&CI.getASTContext(),
                                                TheRewriter);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  void EndSourceFileAction() override {
    const auto &SM = TheRewriter.getSourceMgr();
    TheRewriter.getEditBuffer(SM.getMainFileID())
        .write(llvm::outs());
  }

private:
  Rewriter TheRewriter;
};

} // namespace

static FrontendPluginRegistry::Add<MaybeUnusedAction>
    X("koshkin_n_MaybeUnused_plugin",
      "Marks unused parameters/variables with [[maybe_unused]] attribute");

