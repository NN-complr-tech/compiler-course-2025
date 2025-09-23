#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class MaybeUnusedVisitor final : public RecursiveASTVisitor &lt;
MaybeUnusedVisitor & gt;
{
public:
  explicit MaybeUnusedVisitor(ASTContext * Context, Rewriter & amp; R)
      : Context(Context), TheRewriter(R) {}

  bool VisitFunctionDecl(FunctionDecl * FuncDecl) {
    if (!FuncDecl - &gt; hasBody() || FuncDecl - &gt; isImplicit())
      return true;

    for (unsigned i = 0, e = FuncDecl - &gt; getNumParams(); i & lt; e; ++i) {
      ParmVarDecl *Param = FuncDecl - &gt;
      getParamDecl(i);
      if (Param & amp; &amp; Param - &gt; getName() == "unused" & amp; &amp;
          !Param - &gt; hasAttr & lt; UnusedAttr & gt; ()) {
        const SourceLocation Loc = Param - &gt;
        getBeginLoc();
        if (Loc.isValid()) {
          TheRewriter.InsertText(Loc, "[[maybe_unused]] ",
                                 /*InsertBefore=*/true);
        }
      }
    }

    if (Stmt *Body = FuncDecl - &gt; getBody())
      ProcessLocalVariables(Body);

    return true;
  }

private:
  void ProcessLocalVariables(Stmt * S) {
    if (!S)
      return;

    if (auto *DS = dyn_cast & lt; DeclStmt & gt; (S)) {
      for (auto *D : DS - &gt; decls()) {
        if (auto *VD = dyn_cast & lt; VarDecl & gt; (D)) {
          if (VD - &gt; isLocalVarDecl() & amp; &amp; !VD - &gt;
              isImplicit() & amp; &amp; VD - &gt; getName() == "unused" & amp;
              &amp; !VD - &gt; hasAttr & lt; UnusedAttr & gt; ()) {
            const SourceLocation Loc = VD - &gt;
            getBeginLoc();
            if (Loc.isValid())
              TheRewriter.InsertText(Loc, "[[maybe_unused]] ",
                                     /*InsertBefore=*/true);
          }
        }
      }
    } else {
      for (Stmt *Child : S - &gt; children()) {
        ProcessLocalVariables(Child);
      }
    }
  }

  ASTContext *const Context;
  Rewriter & amp;
  TheRewriter;
};

class MaybeUnusedConsumer : public ASTConsumer {
public:
  explicit MaybeUnusedConsumer(ASTContext *Context, Rewriter &amp; Rewrite)
      : Visitor(Context, Rewrite) {}

  void HandleTranslationUnit(ASTContext &amp; Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  MaybeUnusedVisitor Visitor;
};

class MaybeUnusedAction : public PluginASTAction {
public:
  std::unique_ptr &lt;
  ASTConsumer &gt;
  CreateASTConsumer(CompilerInstance &amp; CI, llvm::StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique & lt;
    MaybeUnusedConsumer & gt;
    (&amp; CI.getASTContext(), TheRewriter);
  }

  bool ParseArgs(const CompilerInstance &amp; CI, const std::vector &lt;
                 std::string & gt; &amp; Args) override {
    return true;
  }

  void EndSourceFileAction() override {
    const auto &amp;
    SM = TheRewriter.getSourceMgr();
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

private:
  Rewriter TheRewriter;
};

} // namespace

static FrontendPluginRegistry::Add &lt;
MaybeUnusedAction & gt;
X("IvashchukVA_lab1_2var",
  "Marks unused parameters/variables with [[maybe_unused]] attribute");