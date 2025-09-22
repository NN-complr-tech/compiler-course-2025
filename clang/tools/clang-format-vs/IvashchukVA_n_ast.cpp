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
    : public clang::RecursiveASTVisitor<MaybeUnusedVisitor> {
public:
  explicit MaybeUnusedVisitor(clang::ASTContext *context, Rewriter &Rewrite)
      : m_context(context), TheRewriter(Rewrite) {}

  bool VisitFunctionDecl(FunctionDecl *FunctionDeclaration) {
    if (!FunctionDeclaration->hasBody() || FunctionDeclaration->isImplicit())
      return true;

    for (unsigned i = 0, NumParams = FunctionDeclaration->getNumParams();
         i < NumParams; ++i) {
      ParmVarDecl *ParamDecl = FunctionDeclaration->getParamDecl(i);
      if (ParamDecl && ParamDecl->getName() == "unused" &&
          !ParamDecl->hasAttr<UnusedAttr>()) {
        SourceLocation Loc = ParamDecl->getBeginLoc();
        if (Loc.isValid())
          TheRewriter.InsertText(Loc, "[[maybe_unused]] ", true);
      }
    }

    Stmt *FunctionBody = FunctionDeclaration->getBody();
    if (FunctionBody)
      processLocalVars(FunctionBody);

    return true;
  }

private:
  void processLocalVars(Stmt *Statement) {
    if (!Statement)
      return;

    if (auto *DeclStatement = dyn_cast<DeclStmt>(Statement)) {
      for (auto *Declaration : DeclStatement->decls()) {
        if (auto *VarDecl = dyn_cast<VarDecl>(Declaration)) {
          if (VarDecl->isLocalVarDecl() && !VarDecl->isImplicit() &&
              VarDecl->getName() == "unused" &&
              !VarDecl->hasAttr<UnusedAttr>()) {
            SourceLocation Loc = VarDecl->getBeginLoc();
            if (Loc.isValid())
              TheRewriter.InsertText(Loc, "[[maybe_unused]] ", true);
          }
        }
      }
    } else {
      for (Stmt *ChildStatement : Statement->children()) {
        processLocalVars(ChildStatement);
      }
    }
  }

  clang::ASTContext *m_context;
  clang::Rewriter &TheRewriter;
};

class MaybeUnusedConsumer final : public clang::ASTConsumer {
public:
  explicit MaybeUnusedConsumer(clang::ASTContext *context, Rewriter &Rewrite)
      : Visitor(context, Rewrite) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  MaybeUnusedVisitor Visitor;
};

class MaybeUnusedAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MaybeUnusedConsumer>(&CI.getASTContext(),
                                                 TheRewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  void EndSourceFileAction() override {
    auto &SM = TheRewriter.getSourceMgr();
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

private:
  Rewriter TheRewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<MaybeUnusedAction>
    X("IvashchukVA_n_ast",
      "Marks unused parameters/variables with [[maybe_unused]] attribute");