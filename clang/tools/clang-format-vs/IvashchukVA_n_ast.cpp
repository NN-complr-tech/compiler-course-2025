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

  bool VisitFunctionDecl(FunctionDecl *FuncDecl) {
    if (!FuncDecl->hasBody() || FuncDecl->isImplicit())
      return true;

    for (unsigned i = 0, e = FuncDecl->getNumParams(); i < e; ++i) {
      ParmVarDecl *Param = FuncDecl->getParamDecl(i);
      if (Param && Param->getName() == "unused" &&
          !Param->hasAttr<UnusedAttr>()) {
        const SourceLocation Loc = Param->getBeginLoc();
        if (Loc.isValid()) {
          TheRewriter.InsertText(Loc, "[[maybe_unused]] ",
                                 /*InsertAfter=*/false);
        }
      }
    }

    if (Stmt *Body = FuncDecl->getBody())
      ProcessLocalVariables(Body);

    return true;
  }

private:
  void ProcessLocalVariables(Stmt *S) {
    if (!S)
      return;

    if (auto *DS = dyn_cast<DeclStmt>(S)) {
      for (auto *D : DS->decls()) {
        if (auto *VD = dyn_cast<VarDecl>(D)) {
          if (VD->isLocalVarDecl() && !VD->isImplicit() &&
              VD->getName() == "unused" && !VD->hasAttr<UnusedAttr>()) {
            const SourceLocation Loc = VD->getBeginLoc();
            if (Loc.isValid())
              TheRewriter.InsertText(Loc, "[[maybe_unused]] ",
                                     /*InsertAfter=*/false);
          }
        }
      }
    } else {
      for (Stmt *Child : S->children()) {
        ProcessLocalVariables(Child);
      }
    }
  }

  ASTContext *const Context;
  Rewriter &TheRewriter;
};

class MaybeUnusedConsumer : public ASTConsumer {
public:
  explicit MaybeUnusedConsumer(ASTContext *Context, Rewriter &Rewrite)
      : Visitor(Context, Rewrite) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  MaybeUnusedVisitor Visitor;
};

class MaybeUnusedAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
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
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

private:
  Rewriter TheRewriter;
};

} // namespace

static FrontendPluginRegistry::Add<MaybeUnusedAction>
    X("IvashchukVA_lab1_2var",
      "Marks unused parameters/variables with [[maybe_unused]] attribute");